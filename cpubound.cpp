/* This is free and unencumbered software released into the public domain.
 * Refer to LICENSE.txt in this directory. */

/* Compute-bound workload that sorts random numbers. */

#include <algorithm>
#include <array>
#include <iostream>
#include <random>

int
main()
{
    std::array<int, 1000000> numbers;
    std::random_device rng_device;
    std::default_random_engine rng(rng_device());
    std::uniform_int_distribution<int> uint_dist;

    for (;;)
    {
        std::cout << "Initialising random numbers..." << std::endl;
        for (auto &number : numbers)
        {
            number = uint_dist(rng);
        }
        std::cout << "Starting sort..." << std::endl;
        std::sort(numbers.begin(), numbers.end());
        std::cout << "Sorted." << std::endl;
    }

    return EXIT_SUCCESS;
}
