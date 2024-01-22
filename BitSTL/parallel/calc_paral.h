/*
 * 并发计算函数库
 */
#ifndef CALC_PARAL_H
#define CALC_PARAL_H

#include <algorithm>

namespace bitstl
{
    template<typename Iterator, typename T>
    struct accumulate_once
    {
        void operator()(Iterator first, Iterator last, T& result)
        {
            result = std::accumulate(first, last, result);
        }
    };

    template<typename Iterator, typename T>
    T accumulate_paral(Iterator first, Iterator last, T init)
    {
        const unsigned long length = std::distance(first, last);
        if (0 == length)
            return init;

        unsigned long const min_per_thread = 20;
        unsigned long const max_threads = (length + min_per_thread - 1) / min_per_thread;
        unsigned long const hardware_threads = std::thread::hardware_concurrency();
        unsigned long const num_threads = std::min(hardware_threads != 0 ? hardware_threads : 2, max_threads);
        unsigned long const true_per_thread = length / num_threads;

        std::vector<T> results(num_threads);
        std::vector<std::thread> threads(num_threads - 1);

        Iterator start = first;
        for (unsigned long i = 0; i < (num_threads - 1); ++i)
        {
            Iterator end = start;
            std::advance(end, true_per_thread);
            threads[i] = std::thread(accumulate_once<Iterator, T>(), start, end, std::ref(results[i]));
            start = end;
        }
        accumulate_once<Iterator, T>()(start, last, results[num_threads - 1]);

        for (auto& entry : threads)
            entry.join();

        return std::accumulate(results.begin(), results.end(), init);
    }
}

#endif // !CALC_PARAL_H


