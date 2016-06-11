#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>


int main()
{
    char readbuf[25];
    int fd = open("/dev/rsa", O_RDWR);

    if (fd<0)
    {
	return errno;
    }
    write(fd, "7",1);
    read(fd, readbuf, 25);

    printf("Value is %s", readbuf);


    return 0;
}
