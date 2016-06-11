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

char* tostring(unsigned long ulong_value)
{
    
    char *buf;
    int c;
    const int n= snprintf(NULL, 0, "%lu", ulong_value);
    if(n <= 0)
    {
	return NULL;
    }

    buf = calloc(n+1,sizeof(char));
    //char buf[n+1];
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

int main()
{
    char *start = "7";
    char *end;
    char *cur;
    long val;
    int i;
    char *tempbuffer;
    char *curTempBuffer;
    char *we;
    int N = 143;
    int e = 23;
    int d = 47;
    char *endstring;
    

    
    /*start = malloc(30);
    if(!start)
    {
	return 1;
	}*/


    
    
    cur = start;
    end = start +strlen(start)+1;
    i = strlen(start);
    printf("length %d\n", i);

    i = end - start;
    printf("I = %d\n", i);
    
    tempbuffer = malloc(end - start + 1 );
    curTempBuffer = tempbuffer;

    i=0;
    do
    {
	printf("%d\n",i++);
	*(curTempBuffer++) = *(cur++);
    }while(cur != end);

    val = strtol(tempbuffer,&we, 10);
    printf("Initial value = %ld\n", val);

    val = calc(val,N,e);
    
    printf("encr Value = %ld\n",val);


    val = calc(val,N,d);
    printf("dec Value = %ld\n",val);



    endstring = tostring(val);
    printf("%s\n",endstring);



    
    //free(start);
    free(tempbuffer);
    free(endstring);
    return 0;
}
