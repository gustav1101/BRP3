#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>


    


int main(int argc, char* argv[])
{

    
    char readbuf[25];
    int fd = open("/dev/rsa", O_RDWR);
    char* message = "7";

    if(argc==2)
    {
	message = argv[1];
	printf("New value is %s\n", argv[1]);
    }

    printf("Starting test...\n");
    
    if (fd<0)
    {
	printf("Error on Test...%s\n", strerror(errno));
	return errno;
    }
    printf("Writing...\n");
    write(fd, message,strlen(message));
    printf("Reading.. \n");
    read(fd, readbuf, 25);

    printf("Value is %s\n", readbuf);


    

    return 0;
}
