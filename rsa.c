#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>


int main()
{
    char readbuf[25];
    int fd = open("/dev/rsa", O_RDWR);

    printf("Starting test...\n");
    
    if (fd<0)
    {
	printf("Error on Test...%s\n", strerror(errno));
	return errno;
    }
    printf("Writing...\n");
    write(fd, "7",1);
    printf("Reading.. \n");
    read(fd, readbuf, 25);

    printf("Value is %s", readbuf);


    return 0;
}
