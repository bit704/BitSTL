/*
 * 并发排序函数库
 */
#ifndef SORT_PARAL_H
#define SORT_PARAL_H

#include <list>
#include <future>
#include <numeric>

#include "threadsafe/stack_ts.h"

namespace bitstl
{
    template<typename T, typename Comp>
    struct sorter;

    template<typename T, typename Comp>
    std::list<T> quick_sort_paral(std::list<T> input, Comp)
    {
        if (input.empty())
            return input;
        static sorter<T,Comp> s;
        return s.sort(input);
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

        sorter() : max_thread_count(std::thread::hardware_concurrency() / 2),
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
                chunk->promise.set_value(do_sort(chunk->data));
        }

        // 若在do_sort中动态增加thread会导致同时有多个thread并发向threads中增加thread，造成不能百分百复现的访问冲突
        std::list<T> sort(std::list<T>& chunk_data)
        {
            while (threads.size() < max_thread_count - 1)
                threads.push_back(std::thread(&sorter<T, Comp>::thread_work, this));
            return do_sort(chunk_data);
        }
        
        std::list<T> do_sort(std::list<T>& chunk_data)
        {
            if (chunk_data.empty() || chunk_data.size() == 1)
                return std::move(chunk_data);

            if (chunk_data.size() == 2)
            {
                if (Comp()(chunk_data.front(), chunk_data.back()))
                    return chunk_data;
                else
                    return { chunk_data.back(), chunk_data.front() };
            }

            std::list<T> result;

            // 取第一个值作为划分轴
            result.splice(result.begin(), chunk_data, chunk_data.begin());
            const T& partition_val = *result.begin();
            typename std::list<T>::iterator pivot = 
                std::partition(chunk_data.begin(), chunk_data.end(), 
                    [&](const T& v) {return Comp()(v, partition_val); });
            
            chunk_to_sort new_lower_chunk;
            new_lower_chunk.data.splice(new_lower_chunk.data.end(), chunk_data, chunk_data.begin(), pivot);
            
            std::future<std::list<T>> new_lower = new_lower_chunk.promise.get_future();
            
            chunks.push(std::move(new_lower_chunk));

            // 当前线程对后半排序
            std::list<T> new_higher(do_sort(chunk_data));
            result.splice(result.end(), new_higher);

            // 其它线程对前半排序
            while (new_lower.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
                try_sort_chunk();

            result.splice(result.begin(), new_lower.get());

            return result;
        }
    };
}
#endif // !SORT_PARAL_H
