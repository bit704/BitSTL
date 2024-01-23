/*
 * 并发算法库
 */
#ifndef ALGO_PARAL_H
#define ALGO_PARAL_H

#include <numeric>
#include <vector>
#include <thread>
#include <future>
#include <cassert>

#include "threadsafe/stack_ts.h"

namespace bitstl
{
    using ulong = unsigned long;

    // 划分子任务
    static void get_partition(const ulong data_length, ulong& thread_num, ulong& data_per_thread)
    {
        assert(data_length > 0);
        const ulong min_per_thread = 20ul;
        const ulong max_threads = (data_length + min_per_thread - 1) / min_per_thread;
        const ulong hardware_threads = std::thread::hardware_concurrency();
        thread_num = std::min(hardware_threads != 0 ? hardware_threads : 2, max_threads);
        data_per_thread = data_length / thread_num;
    }

    template<typename Iterator, typename Func>
    void for_each_paral(Iterator first, Iterator last, Func f)
    {
        ulong const data_length = std::distance(first, last);
        if (!data_length)
            return;

        ulong thread_num = 0, data_per_thread = 0;
        get_partition(data_length, thread_num, data_per_thread);

        std::vector<std::future<void>> futures(thread_num - 1);
        std::vector<std::thread> threads(thread_num - 1);

        Iterator start = first;
        for (ulong i = 0; i < (thread_num - 1); ++i)
        {
            Iterator end = start;
            std::advance(end, data_per_thread);
            std::packaged_task<void(void)> task([=]()
            {
                std::for_each(start, end, f);
            });
            futures[i] = task.get_future();
            threads[i] = std::thread(std::move(task));
            start = end;
        }
        std::for_each(start, last, f);
        for (ulong i = 0; i < (thread_num - 1); ++i)
        {
            futures[i].get();
            threads[i].join();
        }
            
    }

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
        const ulong data_length = std::distance(first, last);
        if (!data_length)
            return init;

        ulong thread_num = 0, data_per_thread = 0;
        get_partition(data_length, thread_num, data_per_thread);

        std::vector<std::future<T>> futures(thread_num - 1);
        std::vector<std::thread> threads(thread_num - 1);

        Iterator start = first;
        for (ulong i = 0; i < (thread_num - 1); ++i)
        {
            Iterator end = start;
            std::advance(end, data_per_thread);
            // 使用{}初始化而不是()初始化避免编译器将其作为函数声明
            std::packaged_task<T(Iterator, Iterator)> task{ accumulate_once<Iterator, T>() };
            futures[i] = task.get_future();
            threads[i] = std::thread(std::move(task), start, end);
            start = end;
        }
        
        T res = init + accumulate_once<Iterator, T>()(start, last);

        for (auto& thread : threads)
            thread.join();

        for (auto& future : futures)
            res += future.get();

        return res;
    }

    template<typename T, typename Comp>
    struct sorter;

    template<typename T, typename Comp>
    std::list<T> quick_sort_paral(const std::list<T>& input, Comp comp)
    {
        if (input.empty())
            return input;
        static sorter<T, Comp> s;
        return s.sort(input, comp);
    }

    template<typename T, typename Comp>
    struct sorter
    {
        struct chunk_to_sort
        {
            std::list<T> data;
            std::promise<std::list<T>> promise;

            chunk_to_sort() = default;
            // promise仅可移动
            chunk_to_sort(chunk_to_sort&& other) : data(std::move(other.data)), promise(std::move(other.promise)) {}
        };
        stack_ts<chunk_to_sort> chunks;

        std::vector<std::thread> threads;
        const unsigned max_thread_count;
        std::atomic_bool end;
        Comp comp;

        sorter() : max_thread_count(std::thread::hardware_concurrency()),
            end(false) {}

        ~sorter()
        {
            end.store(true);
            for (unsigned int i = 0; i < threads.size(); ++i)
                threads[i].join();
        }

        void thread_work()
        {
            while (!end.load())
            {
                try_sort_chunk();
                std::this_thread::yield();
            }
        }

        void try_sort_chunk()
        {
            std::shared_ptr<chunk_to_sort> chunk = chunks.pop();
            if (chunk)
                chunk->promise.set_value(*do_sort(std::make_unique<std::list<T>>(chunk->data)));
        }

        // 若在do_sort中动态增加thread会导致同时有多个thread并发向threads中增加thread，造成不能百分百复现的访问冲突
        std::list<T> sort(std::list<T> chunk_data, Comp _comp)
        {
            comp = _comp;
            while (threads.size() < max_thread_count - 1)
                threads.push_back(std::thread(&sorter<T, Comp>::thread_work, this));
            return *do_sort(std::make_unique<std::list<T>>(chunk_data));
        }

        // 递归太深会导致stack overflow
        std::unique_ptr<std::list<T>> do_sort(std::unique_ptr<std::list<T>> chunk_data)
        {
            if (chunk_data->empty() || chunk_data->size() == 1)
                return std::move(chunk_data);

            std::unique_ptr<std::list<T>> result = std::make_unique<std::list<T>>();

            // 取第一个值作为划分轴
            result->splice(result->begin(), *chunk_data, chunk_data->begin());
            const T& partition_val = *(result->begin());
            typename std::list<T>::iterator pivot =
                std::partition(chunk_data->begin(), chunk_data->end(),
                    [&](const T& v) {return comp(v, partition_val); });

            chunk_to_sort new_lower_chunk;
            new_lower_chunk.data.splice(new_lower_chunk.data.end(), *chunk_data, chunk_data->begin(), pivot);

            std::future<std::list<T>> new_lower = new_lower_chunk.promise.get_future();

            chunks.push(std::move(new_lower_chunk));

            // 当前线程对后半排序
            std::list<T> new_higher(*do_sort(std::move(chunk_data)));
            result->splice(result->end(), new_higher);

            // 其它线程对前半排序
            while (new_lower.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
                try_sort_chunk();

            result->splice(result->begin(), new_lower.get());

            return std::move(result);
        }
    };

    template<typename Iterator, typename MatchType>
    Iterator find_paral(Iterator first, Iterator last, MatchType match)
    {
        struct iteration
        {
            void operator()(
                Iterator begin, Iterator end, MatchType match,
                std::promise<Iterator>* result,
                std::atomic<bool>* done)
            {
                try
                {
                    for (; (begin != end) && !done->load(); ++begin)
                    {
                        if (*begin == match)
                        {
                            result->set_value(begin);
                            done->store(true);
                            return;
                        }
                    }
                }
                catch (...)
                {
                    try
                    {
                        result->set_exception(std::current_exception());
                        done->store(true);
                    }
                    catch (...)
                    {}
                }
            }
        };

        const ulong data_length = std::distance(first, last);
        if (!data_length)
            return last;

        ulong thread_num = 0, data_per_thread = 0;
        get_partition(data_length, thread_num, data_per_thread);

        std::promise<Iterator> result;
        std::atomic<bool> done(false);
        std::vector<std::thread> threads(thread_num - 1);

        Iterator start = first;
        for (ulong i = 0; i < (thread_num - 1); ++i)
        {
            Iterator end = start;
            std::advance(end, data_per_thread);
            threads[i] = std::thread(iteration(), start, end, match, &result, &done);
            start = end;
        }

        iteration()(start, last, match, &result, &done);

        for (auto& thread : threads)
            thread.join();

        if (!done.load())
            return last;

        return result.get_future().get();
    }


}

#endif // !ALGO_PARAL_H


