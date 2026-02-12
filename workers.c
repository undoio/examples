/* This is free and unencumbered software released into the public domain.
 * Refer to LICENSE.txt in this directory. */

/**
 * Multi-threaded worker example. The main thread creates some worker threads
 * and sends requests (in the form of length-prefixed strings) to the workers
 * over the "down" pipe. The workers read requests from the pipe and return
 * results to the main thread over the "up" pipe.
 *
 * This example demonstrates a couple of concurrency bugs:
 *
 * 1. Access to the pipes is not guarded by a mutex, so that multiple workers
 *    might incorrectly interleave reads or writes on the same pipe.
 *
 * 2. select() wakes up all waiting threads when the file descriptor becomes
 *    readable, resulting in a "thundering herd" where all the workers try to read
 *    from the pipe at the same time.
 */

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>

/* Read a length-prefixed packet from fd and update *o_packet with a pointer to
 * the allocated packet. The caller must free the result.
 */
static void
read_packet(char **o_packet, ssize_t *o_length, int fd)
{
    ssize_t length;
    ssize_t r = read(fd, &length, sizeof length);
    assert(r == sizeof length);
    char *result = calloc(1, length);
    assert(result != 0);
    r = read(fd, result, length);
    assert(r == length);
    *o_packet = result;
    *o_length = length;
}

/* Write a length-prefixed packet to fd. */
static void
write_packet(const char *packet, ssize_t length, int fd)
{
    ssize_t r = write(fd, &length, sizeof length);
    assert(r == sizeof length);
    r = write(fd, packet, length);
    assert(r == length);
}

/* The main threads send requests on the "down" pipe and the worker threads
 * return results on the "up" pipe. */
static int down_pipe[2], up_pipe[2];

static void *
worker_thread(void *arg)
{
    for (;;)
    {
        /* Wait for a packet. */
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(down_pipe[0], &readfds);
        int n = select(down_pipe[0] + 1, &readfds, NULL, NULL, NULL);
        assert(n > 0);
        assert(FD_ISSET(down_pipe[0], &readfds));

        /* Read packet. */
        char *packet;
        ssize_t length;
        read_packet(&packet, &length, down_pipe[0]);

        /* Zero length packet is a request to exit. */
        if (!length) return arg;

        /* Write result packet. */
        write_packet(packet, length, up_pipe[1]);
        free(packet);
    }
}

int
main(void)
{
    int r = pipe(down_pipe);
    assert(r == 0);
    r = pipe(up_pipe);
    assert(r == 0);

    /* Create the worker threads. */
    size_t nthreads = 2;
    pthread_t *thread = calloc(nthreads, sizeof *thread);
    assert(thread != NULL);
    for (size_t i = 0; i < nthreads; ++i)
    {
        r = pthread_create(&thread[i], NULL, worker_thread, NULL);
        assert(r == 0);
    }

    /* Distribute some work to the workers and collect the results. */
    for (size_t i = 0; i < 16; ++i)
    {
        ssize_t length1 = 1 << i;
        char *packet1 = calloc(1, length1);
        memset(packet1, 'A' + i % 26, length1);
        write_packet(packet1, length1, down_pipe[1]);
        char *packet2;
        ssize_t length2;
        read_packet(&packet2, &length2, up_pipe[0]);
        assert(length2 == length1);
        assert(0 == memcmp(packet1, packet2, length1));
        free(packet1);
        free(packet2);
        printf("Checked packet, length=%zu\n", length1);
    }

    /* Tell the workers to exit by sending zero-length packets. */
    for (size_t i = 0; i < nthreads; ++i)
    {
        write_packet("", 0, down_pipe[1]);
    }

    /* Wait for the workers to exit. */
    for (size_t i = 0; i < nthreads; ++i)
    {
        r = pthread_join(thread[i], NULL);
        assert(r == 0);
    }
    free(thread);
    return EXIT_SUCCESS;
}
