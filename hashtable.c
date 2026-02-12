/* This is free and unencumbered software released into the public domain.
 * Refer to LICENSE.txt in this directory. */

/* Randomized stress test of an open addressing hash table. */

#include <assert.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

/* Fixed-size closed hash table representing a set of integers. */
typedef struct table table_t;

/* Return empty table with n slots, or NULL if allocation fails. */
table_t *make_table(size_t n, size_t skip);

/* Return true iff element is in the table. */
bool table_contains(const table_t *table, int element);

/* Try to add element to table; return false if table is full. */
bool table_add(table_t *table, int element);

/* Remove element from table if it is present. */
void table_remove(table_t *table, int element);

/* Unused slot in table. */
#define UNUSED INT_MIN

struct table
{
    size_t n;    /* Number of slots in table. */
    size_t skip; /* Slots to skip when traversing hash chain. */
    int slot[0]; /* Array of slots containing elements or UNUSED. */
};

table_t *
make_table(size_t n, size_t skip)
{
    table_t *table = malloc(sizeof *table + n * sizeof table->slot[0]);
    if (table)
    {
        table->n = n;
        table->skip = skip;
        for (size_t i = 0; i < n; ++i)
        {
            table->slot[i] = UNUSED;
        }
    }
    return table;
}

/* Find index of slot containing element, or the index of the empty
 * slot where it should be added. Return true if successful, false if
 * table is full. */
static bool
_table_find(size_t *index, const table_t *table, int element)
{
    size_t i = (size_t)element % table->n;
    size_t j = i;
    assert(element != UNUSED);
    while (table->slot[j] != element && table->slot[j] != UNUSED)
    {
        j = (j + table->skip) % table->n; /* Next slot in hash chain. */
        if (j == i)
        {
            return false;
        }
    }
    *index = j;
    return true;
}

bool
table_contains(const table_t *table, int element)
{
    size_t i;
    return _table_find(&i, table, element) && table->slot[i] == element;
}

bool
table_add(table_t *table, int element)
{
    size_t i;
    if (_table_find(&i, table, element))
    {
        table->slot[i] = element;
        return true;
    }
    return false;
}

void
table_remove(table_t *table, int element)
{
    size_t i;
    if (_table_find(&i, table, element))
    {
        table->slot[i] = UNUSED;
    }
}

/* Randomized stress test. */
int
main(int argc, char **argv)
{
    unsigned seed;
    if (argc == 2)
    {
        seed = strtoul(argv[1], NULL, 10);
    }
    else
    {
        int fd = open("/dev/urandom", O_RDONLY);
        assert(fd >= 0);
        read(fd, &seed, sizeof seed);
        close(fd);
    }
    srand(seed);

    table_t *table = make_table(100, 17);
    assert(table);

    bool indicator[10000] = { false }; /* Expected contents of the table. */
    for (unsigned i = 0; i < sizeof indicator; ++i)
    {
        int element = rand() % sizeof indicator;
        if (indicator[element])
        {
            assert(table_contains(table, element));
            table_remove(table, element);
            indicator[element] = false;
        }
        else
        {
            assert(!table_contains(table, element));
            if (table_add(table, element))
            {
                indicator[element] = true;
            }
        }
    }
    free(table);
    return EXIT_SUCCESS;
}
