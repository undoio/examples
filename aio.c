/* This is free and unencumbered software released into the public domain.
 * Refer to LICENSE.txt in this directory. */

/* Simple asynchronous I/O example using libaio. */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <libaio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void
handle_io_error(int r, const char *s)
{
    if (r < 0)
    {
        errno = -r;
        perror(s);
        exit(EXIT_FAILURE);
    }
}

int
main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s FILENAME\n", argv[0]);
        return EXIT_FAILURE;
    }
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0)
    {
        perror(argv[1]);
        return EXIT_FAILURE;
    }

    io_context_t ctx;
    memset(&ctx, 0, sizeof ctx);
    handle_io_error(io_setup(1, &ctx), "io_setup");

    char buf[4096];
    struct iocb cb;
    memset(&cb, 0, sizeof cb);
    cb.aio_fildes = fd;
    cb.aio_lio_opcode = IO_CMD_PREAD;
    cb.aio_reqprio = 0;
    cb.u.c.buf = buf;
    cb.u.c.nbytes = sizeof buf;
    cb.u.c.offset = 0;
    struct iocb *cbs = &cb;
    handle_io_error(io_submit(ctx, 1, &cbs), "io_submit");

    /* Wait for debugger to attach. */
    printf("pid=%d: press any key to wait for read of %zu bytes from fd %d...\n",
           getpid(), sizeof buf, fd);
    fflush(stdout);
    getchar();

    struct io_event event;
    handle_io_error(io_getevents(ctx, 1, 1, &event, NULL), "io_getevents");
    assert(event.obj == &cb);

    handle_io_error(io_destroy(ctx), "io_destroy");
    close(fd);
    return EXIT_SUCCESS;
}
