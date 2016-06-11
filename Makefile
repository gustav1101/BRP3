obj-m += bin/brpa3_943863.o
CFLAGS_reverse.o += -DDEBUG

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD)/src modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
