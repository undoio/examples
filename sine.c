/* This is free and unencumbered software released into the public domain.
 * Refer to LICENSE.txt in this directory. */

/* Randomized stress test of sin(x). */

#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>

/* Write timestamp and formatted message to stdout. */
static void __attribute__((format(printf, 1, 2)))
log_message(const char *format, ...)
{
    struct timeval tv;
    ssize_t e = gettimeofday(&tv, NULL);
    assert(e == 0);
    struct tm *tm = gmtime(&tv.tv_sec);
    char buffer[sizeof "[HH:MM:SS"];
    ssize_t len = strftime(buffer, sizeof buffer, "[%H:%M:%S", tm);
    assert(len > 0 && len < (ssize_t)sizeof buffer);
    printf("%s.%06lu] ", buffer, tv.tv_usec);
    va_list ap;
    va_start(ap, format);
    vprintf(format, ap);
    va_end(ap);
    putchar('\n');
}

/* Compute sin(x) using a Taylor series. */
static float
sine(float x)
{
    /* This is not the right way to do argument reduction! See K. C. Ng (1993),
     * "Argument reduction for huge arguments". */
    double halfpi = 1.5707963267948966;
    double n = round(x / halfpi);
    double r = x - n * halfpi;
    int m = (int)fmod(n, 4.0f);
    if (m < 0) m += 4;
    assert(0 <= m && m < 4);

    /* Taylor series */
    double t = 1.0;
    unsigned long i = 0;
    if (m % 2 == 0)
    {
        t *= r;
        t /= ++i;
    }
    double a, b = t;
    do
    {
        a = b;
        t *= r;
        t /= ++i;
        t *= r;
        t /= ++i;
        t = -t;
        b += t;
    } while (a != b);
    if (m > 1)
    {
        a = -a;
    }
    return (float)a;
}

/* Check sine() against libc sinf() and log pass/fail. */
static void
check(float x)
{
    float s1 = sinf(x);
    float s2 = sine(x);
    log_message("%s x=%g sinf=%f sine=%f", s1 == s2 ? "pass" : "fail", x, s1, s2);
}

/* Randomized oracle test. */
int
main(int argc, char **argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s ITERATIONS SEED\n", argv[0]);
        return EXIT_FAILURE;
    }
    char *end;
    unsigned long iterations = strtoul(argv[1], &end, 10);
    if (!*argv[1] || *end)
    {
        fprintf(stderr, "Expected ITERATIONS but got %s\n", argv[1]);
        return EXIT_FAILURE;
    }
    unsigned long seed = strtoul(argv[2], &end, 10);
    if (!*argv[2] || *end)
    {
        fprintf(stderr, "Expected SEED but got %s\n", argv[2]);
        return EXIT_FAILURE;
    }
    srand(seed);
    for (unsigned long i = 0; i < iterations; ++i)
    {
        /* Random float using random mantissa and exponent. */
        float m = rand() % (1 << 23);
        int e = rand() % 64 - 55;
        float x = ldexp(m, e);
        check(x);
    }
    return EXIT_SUCCESS;
}
