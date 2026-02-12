/* This is free and unencumbered software released into the public domain.
 * Refer to LICENSE.txt in this directory. */

/* Randomized threaded stress test of a singly-linked list. */

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* A simple element of a list (see struct list below). */
struct element
{
    struct element *next;
    unsigned value;
};

/* A singly-linked list of struct elements. */
struct list
{
    pthread_mutex_t lock;
    struct element *elements;
    unsigned n_elements;
};

static void
list_init(struct list *l)
{
    int r = pthread_mutex_init(&l->lock, NULL);
    assert(!r);
    l->n_elements = 0;
    l->elements = NULL;
}

static void
list_lock(struct list *l)
{
    int r = pthread_mutex_lock(&l->lock);
    assert(!r);
}

static void
list_unlock(struct list *l)
{
    int r = pthread_mutex_unlock(&l->lock);
    assert(!r);
}

/* Allocs memory for element and initializes it to <value>, and returns a pointer to the new
 * element. Does not return NULL (aborts if unable to allocate memory). */
static struct element *
element_new(unsigned value)
{
    struct element *new_el = malloc(sizeof(*new_el));
    if (!new_el)
    {
        fprintf(stderr, "Out of memory allocating %zu bytes\n", sizeof(*new_el));
        abort();
    }

    new_el->value = value;
    return new_el;
}

static void
list_prepend(struct list *l, struct element *new_el)
/* Link element <new_el> at the head of list <l>. */
{
    list_lock(l);
    new_el->next = l->elements;
    l->elements = new_el;
    l->n_elements++;
    list_unlock(l);
}

/* Unlink the nth element from list <l> and return a pointer to it. */
static struct element *
list_unlink(struct list *l, unsigned n)
{
    struct element *ret = NULL;
    struct element **el_pp;
    list_lock(l);
    for (el_pp = &l->elements; (*el_pp) && n--; el_pp = &((*el_pp)->next))
    {
    }

    if (!(*el_pp))
    {
        return NULL; // No nth element.
    }

    ret = (*el_pp);
    (*el_pp) = (*el_pp)->next;
    l->n_elements--;

    list_unlock(l);
    return ret;
}

/* Unlink the <n>th element from list <l1> and link it into list <l2>. */
static void
list_move_element(struct list *l1, unsigned n, struct list *l2)
{
    list_lock(l1);
    list_lock(l2);
    struct element *el = list_unlink(l1, n);
    list_prepend(l2, el);
    list_unlock(l2);
    list_unlock(l1);
}

/* Unlinks all elements from list <l> and frees associated memory. */
static void
list_free(struct list *l)
{
    while (l->elements)
    {
        free(list_unlink(l, 0));
    }
}

/* Some lists which we will use to test the above implementaiton. */
enum
{
    n_lists = 2
};
static struct list g_lists[n_lists];

static void *
tester(void *p)
{
    (void)p;
    unsigned iters = 1000;
    while (iters--)
    {
        printf("iters=%d\n", iters);
        struct list *l2;
        struct list *l = &g_lists[rand() % n_lists];

        switch (rand() % 2) // note: currently only tests add and remove
        {
            case 0: // add
                list_prepend(l, element_new(rand()));
                break;

            case 1: // remove
                if (l->n_elements)
                {
                    list_unlink(l, (rand() % l->n_elements) + 1);
                }
                break;

            case 2: // move
                l2 = &g_lists[rand() % n_lists];
                if (l->n_elements)
                {
                    list_move_element(l, rand() % l->n_elements, l2);
                }
                break;
        }
    }

    return NULL;
}

int
main(void)
{
    int r;
    printf("Running as pid %d\n", getpid());

    struct list *l;
    for (l = g_lists; l < g_lists + n_lists; l++)
    {
        list_init(l);
    }

    pthread_t threads[2];
    unsigned n, n_threads = sizeof(threads) / sizeof(threads[0]);
    for (n = 0; n < n_threads; ++n)
    {
        r = pthread_create(&threads[n], NULL, tester, NULL);
        assert(!r);
    }

    for (n = 0; n < n_threads; ++n)
    {
        r = pthread_join(threads[n], NULL);
        assert(!r);
    }

    for (l = g_lists; l < g_lists + n_lists; l++)
    {
        list_free(l);
    }

    return EXIT_SUCCESS;
}
