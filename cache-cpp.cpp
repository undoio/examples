/* This is free and unencumbered software released into the public domain.
 * Refer to LICENSE.txt in this directory. */

/* Randomized stress test of a sqrt(x) caching mechanism. */

#include <algorithm>
#include <cmath>
#include <deque>
#include <exception>
#include <iostream>
#include <map>
#include <random>
#include <sstream>
#include <vector>

/*
 * Returns a random integer in the range [0, max]
 */
static int
random_int(int max)
{
    /* Just use a deterministic PRNG for demo purposes */
    static std::mt19937 generator;
    static std::uniform_int_distribution<int> distribution(0, max);

    return distribution(generator);
}

/*
 * Cache containing a double ended queue of 100 <number, square root value> pairs
 */
class CacheSqroot
{
public:
    CacheSqroot()
    {
    }
    int operator()(int number);

private:
    const size_t cache_size = 100;
    std::deque<std::pair<unsigned int, unsigned int>> data;
};

/*
 * Searches for number inside the data cache; if found, it returns its cached square root.
 * Otherwise, it stores its square root at the end of the cache queue, doing so also for
 * numbers immediately lower and higher than the argument
 */
int
CacheSqroot::operator()(int number)
{
    auto f = std::find_if(data.begin(), data.end(),
                          [&number](const std::pair<unsigned char, unsigned char> &p) {
                              return p.first == number;
                          });

    if (f != data.end())
    {
        /* Cache hit! */
        return f->second;
    }

    /* Cache miss. Find correct result and populate a few cache entries. */
    int sqroot;
    for (int number_adj = number - 1; number_adj < number + 1; ++number_adj)
    {
        unsigned char sqroot_adj = static_cast<int>(sqrt(number_adj));
        data.emplace_back(number_adj, sqroot_adj);
        while (data.size() > cache_size)
            data.pop_front();

        if (number_adj == number)
        {
            /* This is our return value. */
            sqroot = sqroot_adj;
        }
    }

    return sqroot;
}

/*
 * Class used for evaluating the accuracy of the cache content, by comparing the cached
 * square root of a number, with its calculated (correct) one
 */
class CacheTester
{
public:
    CacheTester(size_t cachesize __attribute__((unused))) {}
    void operator()(int number);

public:
    class CacheFailure : public std::exception
    {
    public:
        CacheFailure(int number, int cached, int correct)
        {
            std::stringstream ss;
            ss << "number=" << number
               << " sqroot_cache=" << cached
               << " sqroot_correct=" << correct;
            details = ss.str();
        }
        const char *what() const noexcept
        {
            return details.c_str();
        }

    private:
        std::string details;
    };

private:
    CacheSqroot sqrooter;
};

void
CacheTester::operator()(int number)
{
    /* Check cache_calculate() with the given number. */
    int sqroot_cache = sqrooter(number);
    int sqroot_correct = static_cast<int>(sqrt(number));

    if (sqroot_cache != sqroot_correct)
    {
        /* cache.calculate() returned incorrect value. */
        throw CacheFailure(number, sqroot_cache, sqroot_correct);
    }
}

int
main()
{
    CacheTester tester(100);

    /* Repeatedly check CacheSqroot calculate(). */
    for (int i = 0;; ++i)
    {
        if (i % 100 == 0)
        {
            std::cout << "i=" << i << "\n";
        }
        /* Check cache_calculate() with a random number. */
        int number = random_int(255);
        tester(number);
    }
}
