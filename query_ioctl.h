#ifndef QUERY_IOCTL_H
#define QUERY_IOCTL_H
#include <linux/ioctl.h>

#define BRPA3_SET_EXPONENT _IOW('q', 3, unsigned short)
#define BRPA3_SET_MODULUS _IOW('q', 4, unsigned short)
#define BRPA3_GET_EXPONENT _IOR('q', 5, unsigned short)
#define BRPA3_GET_MODULUS _IOR('q', 6, unsigned short)
 
#endif
