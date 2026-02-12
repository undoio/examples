/* This is free and unencumbered software released into the public domain.
 * Refer to LICENSE.txt in this directory. */

/* Simple arithmetic program with subroutines. */

#include <stdio.h>
#include <stdlib.h>

int a = 0, b = 1, c = 2;

static int
add(int x)
{
    a += x;
    b += x;
    c += x;
    printf("After +%d a=%d, b=%d, c=%d\n", x, a, b, c);
    return a + b + c;
}

static int
multiply(int x)
{
    a *= x;
    b *= x;
    c *= x;
    printf("After x%d a=%d, b=%d, c=%d\n", x, a, b, c);
    return a + b + c;
}

int
main(void)
{
    int x, y, z;
    a++;
    b++;
    c++;
    x = add(2);
    a++;
    b++;
    c++;
    y = multiply(3);
    z = y * y;
    printf("x, y, z now %d, %d, %d\n", x, y, z);

    return EXIT_SUCCESS;
}
