/*
 * 并发计算函数库
 */
#ifndef CALC_PARAL_H
#define CALC_PARAL_H

#include <numeric>
#include <vector>
#include <thread>
#include <future>

namespace bitstl
{
    template<typename Iterator, typename T>
    struct accumulate_once
    {
        T operator()(Iterator first, Iterator last)
        {
            return std::accumulate(first, last, T());
        }
    };

    template<typename Iterator, typename T>
    T accumulate_paral(Iterator first, Iterator last, T init)
    {
        const unsigned long length = std::distance(first, last);
        if (!length)
            return init;

        unsigned long const min_per_thread = 20;
        unsigned long const max_threads = (length + min_per_thread - 1) / min_per_thread;
        unsigned long const hardware_threads = std::thread::hardware_concurrency();
        unsigned long const num_threads = std::min(hardware_threads != 0 ? hardware_threads : 2, max_threads);
        unsigned long const true_per_thread = length / num_threads;

        std::vector<std::future<T>> futures(num_threads - 1);
        std::vector<std::thread> threads(num_threads - 1);

        Iterator start = first;
        for (unsigned long i = 0; i < (num_threads - 1); ++i)
        {
            Iterator end = start;
            std::advance(end, true_per_thread);
            // 使用{}初始化而不是()初始化避免编译器将其作为函数声明
            std::packaged_task<T(Iterator, Iterator)> task{ accumulate_once<Iterator, T>() };
            futures[i] = task.get_future();
            threads[i] = std::thread(std::move(task), start, end);
            start = end;
        }
        
        T res = init + accumulate_once<Iterator, T>()(start, last);

        for (auto& entry : threads)
            entry.join();

        for (auto& future : futures)
            res += future.get();

        return res;
    }
}

#endif // !CALC_PARAL_H


