/* This is free and unencumbered software released into the public domain.
 * Refer to LICENSE.txt in this directory. */

/* Randomized threaded stress test of a producer-consumer queue. */

#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

struct list
{
    struct list *next;
    int val;
};

static struct list *g_list;
static unsigned g_items;
static unsigned g_transactions;
static bool g_done;

static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_condition = PTHREAD_COND_INITIALIZER;

static void *
eater(void *p)
{
    (void)p;
    while (!g_done)
    {
        int e;
        e = pthread_mutex_lock(&g_mutex);
        assert(!e);

        if (g_items)
        {
            /* Consume an item from the list. */
            struct list *item = g_list;
            g_list = g_list->next;
            free(item);
            --g_items;
        }
        else
        {
            /* No items; wait on a condition variable until there are. */
            int r;
            do
            {
                struct timeval tv;
                e = gettimeofday(&tv, NULL);
                assert(!e);
                struct timespec t = { .tv_sec = tv.tv_sec + 1,
                                      .tv_nsec = 1000 * tv.tv_usec };
                r = pthread_cond_timedwait(&g_condition, &g_mutex, &t);
                assert(r == 0 || r == ETIMEDOUT);
            } while (r == ETIMEDOUT && !g_done);
        }
        ++g_transactions;
        e = pthread_mutex_unlock(&g_mutex);
        assert(!e);
    }
    return NULL;
}

static void *
feeder(void *p)
{
    (void)p;
    while (!g_done)
    {
        usleep(1 + (random() % 100 * 1000));

        /* Post an item onto the list. */
        int e;
        e = pthread_mutex_lock(&g_mutex);
        assert(!e);

        struct list *el = malloc(sizeof(*el));
        el->next = g_list;
        el->val = rand();
        g_list = el;
        ++g_items;
        ++g_transactions;

        e = pthread_mutex_unlock(&g_mutex);
        assert(!e);

        /* Notify anyone waiting for a new item. */
        e = pthread_cond_signal(&g_condition);
        assert(!e);
    }
    return NULL;
}

int
main(int argc, char *argv[])
{
    unsigned int duration_s = 10;
    if (argc >= 2)
    {
        duration_s = strtoul(argv[1], NULL, 10);
    }

    /* Spawn the producer and consumer, wait 10s, then wait for them to exit, then finish. */
    pthread_t p1;
    int r = pthread_create(&p1, NULL, eater, NULL);
    assert(r == 0);
    pthread_t p2;
    r = pthread_create(&p2, NULL, feeder, NULL);
    assert(r == 0);

    printf("main:sleep\n");
    sleep(duration_s);

    g_done = true;

    pthread_join(p1, NULL);
    pthread_join(p2, NULL);

    printf("main: finished after completing %u transactions\n", g_transactions);
    return EXIT_SUCCESS;
}
