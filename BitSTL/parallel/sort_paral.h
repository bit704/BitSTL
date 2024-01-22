/*
 * 并发排序函数库
 */
#ifndef SORT_PARAL_H
#define SORT_PARAL_H

#include <list>
#include <future>

namespace bitstl
{
    // TODO 使用线程池
    template<typename T, typename Comp>
    std::list<T> quick_sort_paral(std::list<T> input, Comp comp)
    {
        if (input.empty())
        {
            return input;
        }

        std::list<T> result;
        result.splice(result.begin(), input, input.begin());

        const T& pivot = *result.begin();
        auto divide_point = std::partition(input.begin(), input.end(),
            [&](T const& t) {return comp(t, pivot); });

        std::list<T> lower_part;
        lower_part.splice(lower_part.end(), input, input.begin(), divide_point);

        std::future<std::list<T>> new_lower(std::async(std::launch::async, &quick_sort_paral<T, Comp>, std::move(lower_part), comp));

        std::list<T> new_higher(quick_sort_paral(std::move(input), comp));

        result.splice(result.end(), new_higher);
        result.splice(result.begin(), new_lower.get());
        return result;
    }
}
#endif // !SORT_PARAL_H
