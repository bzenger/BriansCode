#include <stdio.h>
#include <standards.h>
#include <sys/endian.h>

void main()
{
#if __BYTE_ORDER == __BIG_ENDIAN
    printf("byte order is defined as %d\n", __BYTE_ORDER);
    if ( __BYTE_ORDER == __BIG_ENDIAN ) {
	printf(" we have big endian\n");
    }
#endif
    printf("done\n");
}
