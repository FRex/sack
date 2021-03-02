#include <stdio.h>

#define SACK_IMPLEMENTATION
#include "sack.h"

int main(int argc, char ** argv)
{
    int i;
    sack sack;
    sack_init(&sack, 1000);

    puts("passing 1 arg makes it free all pointers but leak the table");
    puts("passing 2 args makes it free everything"); 

    for(i = 0; i < 123; ++i)
        printf("%p\n", sack_memdupmore(&sack, sack_alloc(&sack, 123), 123, 512));

    if(argc > 1)
        sack_freepointers(&sack);

    if(argc > 2)
        sack_deinit(&sack);

    return 0;
}
