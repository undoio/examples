/* This is free and unencumbered software released into the public domain.
 * Refer to LICENSE.txt in this directory. */

#include <stdio.h>
#include <stdlib.h>

static void
increment_pointed_value(int *ptr_to_value_to_increment)
{
    int old = *ptr_to_value_to_increment;
    *ptr_to_value_to_increment += 1;
    printf("Incremented value from %d to %d\n", old, *ptr_to_value_to_increment);
}

int
main(void)
{
    int *ptr = malloc(sizeof(int));

    *ptr = 42;
    printf("Set the value to %d\n", *ptr);

    increment_pointed_value(ptr);

    free(ptr);

    return EXIT_SUCCESS;
}
