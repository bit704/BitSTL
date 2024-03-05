/*
 * 算法库
 */
#ifndef ALGORITHM_H
#define ALGORITHM_H

namespace bitstl
{
    // 从first开始填充count个元素
    template<typename OutputIterator, class Size, class T>
    OutputIterator fill_n(OutputIterator first, Size count, const T& value)
    {
        for (Size i = 0; i < count; ++i)
            *first++ = value;
        return first;
    }
}

#endif // !ALGORITHM_H

