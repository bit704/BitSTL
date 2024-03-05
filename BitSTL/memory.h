/*
 * 动态内存管理库  
 */
#ifndef MEMORY_H
#define MEMORY_H

#include <memory>
#include "config.h"
#include "type_traits.h"
#include "algorithm.h"
#include "iterator.h"

namespace bitstl
{
    // 构造
    template<typename T, typename... Args>
    constexpr T* construct_at(T* p, Args&&... args)
    {
        return ::new (static_cast<void*>(p)) T(bitstl::forward<Args>(args)...);
    }

    // 析构
    template<typename T>
    constexpr void destroy_at(T* p)
    {
        p->~T();
    }

    /*
     * 从first开始填充count个未初始化的元素
     */
    // 元素的拷贝赋值函数为trivial
    template <class ForwardIterator, class Size, class T>
    ForwardIterator unchecked_uninit_fill_n(ForwardIterator first, Size count, const T& value, std::true_type)
    {
        return bitstl::fill_n(first, count, value);
    }

    // 元素的拷贝赋值函数不为trivial
    template <class ForwardIterator, class Size, class T>
    ForwardIterator unchecked_uninit_fill_n(ForwardIterator first, Size count, const T& value, std::false_type)
    {
        auto cur = first;
        try
        {
            while (count--)
            {
                ++cur;
                construct_at(&*cur, value); // &*可以从智能指针取得原始指针
            }
        }
        catch (...)
        {
            for (; first != cur; ++first)
                destroy_at(&*first);
        }
        return cur;
    }

    // 根据元素的拷贝赋值函数是否为trivial的分情况讨论
    template <class ForwardIterator, class Size, class T>
    ForwardIterator uninitialized_fill_n(ForwardIterator first, Size count, const T& value)
    {
        // std::is_trivially_copy_assignable使用内建函数实现
        return bitstl::unchecked_uninit_fill_n(first, count, value,
            std::is_trivially_copy_assignable<typename bitstl::iterator_traits<ForwardIterator>::value_type>{});
    }
}

#endif // !MEMORY_H




