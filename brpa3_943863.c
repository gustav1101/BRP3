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
static unsigned short e = 23;
static unsigned short N = 143;


//make buffer size changeable by user (and stick some description to it)
module_param(buffer_size, ulong, (S_IRUSR | S_IRGRP | S_IROTH));
MODULE_PARM_DESC(buffer_size, "Internal buffer size");















/*
 * Buffer to write in. Also holds a wait queue head for programs to
 * synchronise on this object.
 */
struct buffer {
    wait_queue_head_t read_queue;  //For programs to wait until we're done encrypting (or have something to encrypt)
    char *data;   //Pointer to beginning of buffer
    char *end;    //Pointer to first bit AFTER(!) end of buffer
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
***************ACTUAL MAGIC **********************
*/




/************* BUFFER CONTENT MANIPULATIONS **************/
/*static char *reverse_word(char *start, char *end)
{
    char *orig_start = start, tmp;
    for (; start < end; start++, end--)
    {
	tmp = *start;
	*start = *end;
	*end = tmp;
    }
    return orig_start;
}


static char *reverse_phrase(char *start, char *end)
{
    char *word_start = start, *word_end = NULL;
    while ((word_end = memchr(word_start, ' ', end - word_start)) != NULL) {
	reverse_word(word_start, word_end - 1);
	word_start = word_end + 1;
    }
    reverse_word(word_start, end);
    return reverse_word(start, end);
}
*/

static unsigned long parse(char *start, char *end)
{
    
    char *tempbuffer;
    char *curTempBuffer;
    char *cur;
    unsigned long val;
    int errno;
    int i;

    if(!start)
    {
	printk(KERN_INFO "Start is null\n");
	return 0;
    }

    if(!end)
    {
	printk(KERN_INFO "End is null\n");
	return 0;
    }
    
    if(start > end)
    {
	printk(KERN_INFO "End greater than start\n");
	return 0;
    }
    if(start == end)
    {
	printk(KERN_INFO "End equal to start\n");
	return 0;
    }    
    
    tempbuffer = kzalloc(end - start + 1 , GFP_KERNEL);
    if(unlikely(!tempbuffer))
    {
	return 0;
    }

    curTempBuffer = tempbuffer;
    
    

    cur = start;

    
    //while(cur!=end-1)
    
    for(i = 0; i<(end-start); i++)
    {
	*(curTempBuffer++) = *(cur++);
    }
    
    errno = kstrtoul(tempbuffer, 10, &val );
    if(errno != 0)
    {
	return 0;
    }
    
    kfree(tempbuffer);
    

    return val;

}

static unsigned long calc(unsigned long val, int N, unsigned short depth)
{
    long ret;
   	
    if(depth == 1)
    {
	return val;
    }

    ret = ( (calc(val,N, depth-1) % N) * (val % N) )%N;
    
    return ret;
}



char* tostring(unsigned long ulong_value)
{
    
    char *buf;
    int c;
    const int n= snprintf(NULL, 0, "%lu", ulong_value);
    if(n <= 0)
    {
	return NULL;
    }

    buf = kzalloc(n+1,GFP_KERNEL);
    if(unlikely(!buf))
    {
	return NULL;
    }
    
    c = snprintf(buf, n+1, "%lu", ulong_value);
    if(buf[n]!='\0')
    {
	return NULL;
    }
    if( c!= n)
    {
	return NULL;
    }

    return buf;
}

void rsa_encrypt(char *start, char *end)
{
    char *buf;
    unsigned long val;
    if(start > end)
    {
	return;
    }
    
    val = parse(start,end);
    if(val==0)
    {
	return;
    }

    	
    val = calc(val,N,e);

    printk("Reached this point...\n");
    printk(KERN_INFO "ENC val is %lu\n",val);
	
    
    buf = tostring(val);
    if(unlikely(!buf))
    {
	return;
    }

    
    strncpy(start,buf,(end-start));
    


    kfree(buf);

}





/************** BUFFER OPERATIONS ********************/


static int rsa_open(struct inode *inode, struct file *file)
{
    struct buffer *buf;  //the buffer to be used
    int err = 0;  //return value for this method
/*
 * Real code can use inode to get pointer to the private
 * device state.
 */
    buf = buffer_alloc(buffer_size);  //initialise our buffer
    if (unlikely(!buf)) {             //if that failed: return errornumber
	err = -ENOMEM;
	goto out;
    }
    //store the buffer in the kernel data structure
    file->private_data = buf;
out:
    return err;
}



/*
 * Read from buffer. Copies the data to user space. Returns Error number or number of bytes read.
 */
static ssize_t rsa_read(struct file *file, char __user * out,
			    size_t size, loff_t * off)
{
    struct buffer *buf = file->private_data; //get pointer to buffer
    ssize_t result;  //return value. will contain error number or number of bytes read
    
    while (buf->read_ptr == buf->end) {
	//if no data: return error number
	if (file->f_flags & O_NONBLOCK) {
	    result = -EAGAIN;
	    goto out;
	}
	//wait until something is available to read
	if (wait_event_interruptible
	    (buf->read_queue, buf->read_ptr != buf->end)) {
	    result = -ERESTARTSYS;  //will restart system call
	    goto out;
	}
    }

    //copy data to user space
    size = min(size, (size_t) (buf->end - buf->read_ptr));
    if (copy_to_user(out, buf->read_ptr, size)) {
	result = -EFAULT;  //on failed copying
	goto out;
    }

    //increase buffer position
    buf->read_ptr += size;
    //all was well, return value will contain number of bytes read
    result = size;
out:
    return result;
}


/*
 * Write data in buffer
 */
static ssize_t rsa_write(struct file *file, const char __user * in,
			     size_t size, loff_t * off)
{
    
    struct buffer *buf = file->private_data; //get pointer to buffer
    ssize_t result;  //return value. will contain error number or number of bytes written

    //check buffer size vs size of given data, if it's too big start throwing error numbers.
    if (size > buffer_size) {
	result = -EFBIG;
	goto out;
    }

    //copy data from user space, if failed: return error number
    if (copy_from_user(buf->data, in, size))
    {
	result = -EFAULT;
	goto out;
    }

    //initialise buffer end and current position (to begin of buffer)
    buf->end = buf->data + size;
    buf->read_ptr = buf->data;

    //if there is something to do: encrypt
    if (buf->end > buf->data)
    {
        rsa_encrypt(buf->data, buf->end);
    }
    // wake up possible readers (or other writers)
    wake_up_interruptible(&buf->read_queue);
    //return number of bytes written
    result = size;
out:
    return result;
}


/*
 * Clean up
 */
static int rsa_close(struct inode *inode, struct file *file)
{
    //free buffer
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
    misc_deregister(&rsa_misc_device);
    printk(KERN_INFO "RSA device has been unregistered\n");
}


//link init and exit procedures
module_init(rsa_init);
module_exit(rsa_exit);
