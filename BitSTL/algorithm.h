/*
 * 算法库
 */
#ifndef ALGORITHM_H
#define ALGORITHM_H

#include "memory.h"

namespace bitstl
{
    /*
     * swap函数
     */
    template<typename T>
    constexpr inline void swap(T& lhs, T& rhs)
    {
        auto tmp(move(lhs));
        lhs = move(rhs);
        rhs = move(tmp);
    }

    template<typename T, size_t N>
    constexpr inline void swap(T(&a)[N], T(&b)[N])
    {
        for (size_t n = 0; n < N; ++n)
            swap(a[n], b[n]);
    }

    // 从first开始填充count个元素
    template<typename OutputIterator, typename Size, typename T>
    OutputIterator fill_n(OutputIterator first, Size count, const T& value)
    {
        for (Size i = 0; i < count; ++i)
            *first++ = value;
        return first;
    }

    // 从first到last填充元素
    template<typename ForwardIterator, typename T>
    void fill(ForwardIterator first, ForwardIterator last, const T& value)
    {
        for (; first != last; ++first)
            *first = value;
    }

    // 将first至last的元素复制到以dest_first开始的内存中
    template<typename InputIterator, typename OutputIterator>
    OutputIterator copy(InputIterator first, InputIterator last, OutputIterator dest_first)
    {
        // (void)用于避免逗号运算符被重载的情况
        for (; first != last; (void)++first, (void)++dest_first)
            *dest_first = *first;

        return dest_first;
    }

    // 根据条件pred将first至last的元素复制到以dest_first开始的内存中
    template<typename InputIterator, typename OutputIterator, typename UnaryPredicate>
    OutputIterator copy_if(InputIterator first, InputIterator last, OutputIterator dest_first, UnaryPredicate pred)
    {
        for (; first != last; ++first)
        {
            if (pred(*first))
            {
                *dest_first = *first;
                ++dest_first;
            }
        }
        return dest_first;
    }

    // 将first至last的元素移动到以dest_first开始的内存中
    template<typename InputIterator, typename OutputIterator>
    OutputIterator move(InputIterator first, InputIterator last, OutputIterator dest_first)
    {
        for (; first != last; (void)++first, (void)++dest_first)
            *dest_first = move(*first);

        return dest_first;
    }
}

#endif // !ALGORITHM_H

