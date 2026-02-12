/* This is free and unencumbered software released into the public domain.
 * Refer to LICENSE.txt in this directory. */

/**
 * \brief The worst linked list in the world
 *
 * An example of a thread-unsafe linked list being used to pass
 * blocks of data between threads. Because the threads spend most
 * of their time doing computation (vs manipulating the linked list)
 * this example only fails rarely when run natively.
 */

#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/**
 * \brief Trivial singly linked list structure
 */
struct list
{
    struct list *next;
    void *data;
};

typedef struct list list_t;

static list_t list_head;

static int iterations = 0;

/**
 * \brief Buggy function to push to head of a list
 *
 * This function is subject to a race where `list_head.next`
 * can be updated between when it is read and when it is updated.
 */
static void
s_push(list_t *item)
{
    /* Pairs with assignment to list_head.next */
    list_t *tmp = __atomic_load_n(&list_head.next, __ATOMIC_ACQUIRE);
    /* Thread switch here is bad */
    item->next = tmp;
    /* Thread switch here is bad */
    __atomic_store_n(&list_head.next, item, __ATOMIC_RELEASE);
}

/**
 * \brief Buggy function to pop from the head of a list
 *
 * Similar to the pop function, the `list_head.next` may be updated
 * after `item` has been read.
 */
static void *
s_pop(void)
{
    /* Pairs with the assignment to list_head.next */
    list_t *item = __atomic_load_n(&list_head.next, __ATOMIC_ACQUIRE);
    /* Thread switch here is bad */
    if (!item)
    {
        return NULL;
    }
    /* Thread switch here is bad */
    __atomic_store_n(&list_head.next, item->next, __ATOMIC_RELEASE);
    return item;
}

/**
 * \brief Consume CPU cycles / run up the bbcount
 *
 * We use a busy function to simulate an application where threads mostly
 * do work independently and only rarely synchronise. If the compute time
 * is variable enough (and in practice it often is) then only one thread at
 * a time runs the synchronisation routine, meaning it can have race conditions
 * which go unnoticed (or are very hard to reproduce).
 */
static void
s_compute_junk(void)
{
    long p = 0;
    unsigned long q = 0;
    const int i = iterations;
    for (; p < i; ++p)
    {
        __atomic_fetch_add(&q, p, __ATOMIC_RELAXED);
    }
}

/**
 * \brief Push items onto list between lots of computation
 */
static void *
s_producer(void *arg __attribute__((unused)))
{
    while (true)
    {
        /* Allocate a block of data and submit it to the list */
        list_t *item = malloc(sizeof *item);
        s_push(item);
        for (int i = 0; i < 10; ++i)
        {
            s_compute_junk();
        }
    }
    return NULL;
}

/**
 * \brief Pop items from list between doing lots of computation
 */
static void *
s_consumer(void *arg __attribute__((unused)))
{
    while (true)
    {
        list_t *item = s_pop();
        if (!item)
        {
            /* We've consumed all we can, spin */
            s_compute_junk();
        }
        else
        {
            /* A real application would do some work on the data it
             * got from the list, we just free it.
             */
            free(item);
        }
    }
    return NULL;
}

int
main(int argc, char **argv)
{
    if (argc != 2)
    {
        /* On a typical thinkpad 10000 is a good default value */
        fprintf(stderr, "use: %s iterations (10000 is a good value to try)\n", argv[0]);
        return 1;
    }
    iterations = atoi(argv[1]);
    pthread_t t_producer, t_consumer;
    pthread_create(&t_producer, NULL, s_producer, NULL);
    pthread_create(&t_consumer, NULL, s_consumer, NULL);
    pthread_join(t_producer, NULL);
    return 0;
}
