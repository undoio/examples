/* This is free and unencumbered software released into the public domain.
 * Refer to LICENSE.txt in this directory. */

/* Stack corruption example caused by buffer overflow. */

#include <stdlib.h>
#include <string.h>

static void
foo(void)
{
    int b[1];
    /* Deliberately write past the bound of the array to corrupt the stack. */
    memset(b, 0, 100);
}

int
main(void)
{
    foo();
    return EXIT_SUCCESS;
}
