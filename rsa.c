#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

#include "query_ioctl.h"
    


int main(int argc, char* argv[])
{

    
    char readbuf[25];
    int fd = open("/dev/rsa", O_RDWR);
    char* message = "7";
    unsigned short io;

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

    io = 3;
    if (ioctl(fd, BRPA3_SET_EXPONENT, &io) == -1)
    {
	perror("Failed to set EXPONENT");
    }
    else
    {
	printf("EXPONENT set to  %hu\n", io);
    }

    io = 27;
    if (ioctl(fd, BRPA3_SET_MODULUS, &io) == -1)
    {
	perror("Failed to set MODULUS");
    }
    else
    {
	printf("MODULUS set to  %hu\n", io);
    }
    
    if (ioctl(fd, BRPA3_GET_EXPONENT, &io) == -1)
    {
	perror("Failed to read Exponent");
    }
    else
    {
	printf("Exponent currently is %hu\n", io);
    }


    if (ioctl(fd, BRPA3_GET_MODULUS, &io) == -1)
    {
	perror("Failed to read MODULUS");
    }
    else
    {
	printf("MODULUS currently is %hu\n", io);
    }


//#define BRPA3_GET_MODULUS _IOR('q', 4, unsigned short)
    
    printf("Writing...\n");
    write(fd, message,strlen(message));
    printf("Reading.. \n");
    read(fd, readbuf, 25);

    printf("Value is %s\n", readbuf);


    

    return 0;
}
