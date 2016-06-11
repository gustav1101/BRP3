#include <linux/init.h>	/* __init and __exit macroses */
#include <linux/kernel.h>	/* KERN_INFO macros */
#include <linux/module.h>	/* required for all kernel modules */
#include <linux/moduleparam.h>	/* module_param() and MODULE_PARM_DESC() */
#include <linux/fs.h>	/* struct file_operations, struct file */
#include <linux/miscdevice.h>	/* struct miscdevice and misc_[de]register() */
#include <linux/mutex.h>	/* mutexes */
#include <linux/string.h>	/* memchr() function */
#include <linux/slab.h>	/* kzalloc() function */
#include <linux/sched.h>	/* wait queues */
#include <linux/uaccess.h>	/* copy_{to,from}_user() */

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lukas Kalde <lkalde@uos.de>");
MODULE_DESCRIPTION("In-kernel RSA encryption");

//default buffer size
static unsigned long buffer_size = 8192;

//make buffer size changeable by user (and stick some description to it)
module_param(buffer_size, ulong, (S_IRUSR | S_IRGRP | S_IROTH));
MODULE_PARM_DESC(buffer_size, "Internal buffer size");


/*
 * Buffer to write in. Also holds a wait queue head for programs to
 * synchronise on this object.
 */
struct buffer {
    wait_queue_head_t read_queue;  //For programs to wait until we're done encrypting (or have something to encrypt)
    char *data, *end;   //Pointer to end and beginning of buffer
    char *read_ptr;     //points to current position
    unsigned long size;  //holds size of buffer (not strictly necessary)
};


/*
 * Allocates Buffer. Returns null if failed.
 *
 */
static struct buffer *buffer_alloc(unsigned long size)
{
    //declare, allocate and initialise buffer with zeros
    struct buffer *buf = NULL;
    buf = kzalloc(sizeof(*buf), GFP_KERNEL);
    //if allocation failed: return null
    if (unlikely(!buf))
    {
	goto out;
    }

    //allocate space for the actual buffer inside the buffer struct (GFP_KERNEL: normal memory)
    buf->data = kzalloc(size, GFP_KERNEL);
    //if alloc failed: return null, but free buffer object before that
    if (unlikely(!buf->data))
    {
	goto out_free;
    }

    //initialise waitqueue on the buffer to avoid reading/writing on empty or currently in-use buffers
    init_waitqueue_head(&buf->read_queue);
    /* It's unused for now, but may appear useful later */
    buf->size = size;
out:
    //Either all good here or we failed on trying to initialise the buffer struct
    return buf;
out_free:
    //failed to initialise the buffer inside the buffer struct. Clean up and return null.
    kfree(buf);
    return NULL;
}


/*
 * Clean up and free buffer struct
 */
static void buffer_free(struct buffer *buffer)
{
    kfree(buffer->data);
    kfree(buffer);
}


/*
***************ACTUAL CODE **********************
*/


static ssize_t reverse_write(struct file *file, const char __user * in,
			     size_t size, loff_t * off)
{
    struct buffer *buf = file->private_data;
    ssize_t result;
    if (size > buffer_size) {
	result = -EFBIG;
	goto out;
    }
    if (mutex_lock_interruptible(&buf->lock)) {
	result = -ERESTARTSYS;
	goto out;
    }
    if (copy_from_user(buf->data, in, size)) {
	result = -EFAULT;
	goto out_unlock;
    }
    buf->end = buf->data + size;
    buf->read_ptr = buf->data;
    if (buf->end > buf->data)
	reverse_phrase(buf->data, buf->end - 1);
    wake_up_interruptible(&buf->read_queue);
    result = size;
out_unlock:
    mutex_unlock(&buf->lock);
out:
    return result;
}


static int reverse_close(struct inode *inode, struct file *file)
{
    struct buffer *buf = file->private_data;
    buffer_free(buf);
    return 0;
}








/*
*************** LINKING STUFF **********************
 */


/*
 * Link callbacks to methods
 */
static struct file_operations rsa_fops = {
    .owner = THIS_MODULE,
    .open = rsa_open,
    .read = rsa_read,
    .write = rsa_write,
    .release = rsa_close,
    .llseek = noop_llseek
};



/*
 * misc defice descriptor
 */
static struct miscdevice rsa_misc_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "rsa",
    .fops = &rsa_fops
};

/*
 * On initialisation: Check parameter buffer_size, register the misc device,
 * and print on kernel log that we registered this module
 */
static int __init rsa_init(void)
{
    if (!buffer_size)
    {
	return -1;
    }

    //register the misc char device, point to our struct
    misc_register(&rsa_misc_device);

    
    printk(KERN_INFO
	   "RSA device has been registered, buffer size is %lu bytes\n",
	   buffer_size);
    return 0;
}


/*
 * clean up. Unregister misc device, print message on kernel log.
 */
static void __exit rsa_exit(void)
{
    misc_deregister(&reverse_misc_device);
    printk(KERN_INFO "reverse device has been unregistered\n");
}


//link init and exit procedures
module_init(rsa_init);
module_exit(rsa_exit);
