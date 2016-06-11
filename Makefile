CONFIG_MODULE_SIG=n

obj-m += brpa3_943863.o
CFLAGS_reverse.o += -DDEBUG

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	$(CC) rsa.c -o rsa
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
