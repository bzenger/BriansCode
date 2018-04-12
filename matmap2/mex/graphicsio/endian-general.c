#include <stdio.h>
int little_endian(void);

void main()
{
    if ( little_endian() == 1 ) 
    {
    printf("byte order is little endian\n");
    } else {
    printf("byte order is big endian\n");
    }
}

int little_endian(void)
{
    int x = 1;
    return (x == *((char*)&x));
}
