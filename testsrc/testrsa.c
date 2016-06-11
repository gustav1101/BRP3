#include <stdio.h>
#include <string.h>
#include <stdlib.h>
 
long calc(long start, int N, short depth)
{
    long ret;
    if(depth == 1)
    {
	return start;
    }

    ret = ( (calc(start,N, depth-1) % N) * (start % N) )%N;
    
    return ret;
}

int main()
{
    unsigned long val = 7   ;
    int i;
    unsigned long start;

    int e = 23;
    int d = 47;
    int N = 143;

    start = val;
    
    val = calc(start, N, e);
    printf("%ld\n",val);

    val = calc(val, N, d);
    printf("%ld\n",val);
    

    
    
    return 0;
}


