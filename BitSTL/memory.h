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
        return ::new (static_cast<void*>(p)) T(forward<Args>(args)...);
    }

    // 析构
    template<typename T>
    constexpr void destroy_at(T* p)
    {
        p->~T();
    }

    // 从first开始析构n个
    template<typename ForwardIterator, typename Size >
    constexpr ForwardIterator destroy_n(ForwardIterator first, Size n)
    {
        for (; n > 0; (void)++first, --n)
            std::destroy_at(std::addressof(*first));
        return first;
    }

    // 析构first到last的元素
    template<typename ForwardIterator >
    constexpr void destroy(ForwardIterator first, ForwardIterator last)
    {
        for (; first != last; ++first)
            std::destroy_at(std::addressof(*first));
    }
   
    /*
     * 从first开始在已分配的内存中填充count个的元素
     */
    // 元素的拷贝赋值函数为trivial
    template<typename ForwardIterator, typename Size, typename T>
    ForwardIterator unchecked_uninit_fill_n(ForwardIterator first, Size count, const T& value, std::true_type)
    {
        return fill_n(first, count, value);
    }

    // 元素的拷贝赋值函数不为trivial
    template<typename ForwardIterator, typename Size, typename T>
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
    template<typename ForwardIterator, typename Size, typename T>
    ForwardIterator uninitialized_fill_n(ForwardIterator first, Size count, const T& value)
    {
        // std::is_trivially_copy_assignable使用内建函数实现
        return bitstl::unchecked_uninit_fill_n(first, count, value,
            std::is_trivially_copy_assignable<typename bitstl::iterator_traits<ForwardIterator>::value_type>{});
    }

    /*
     * 从first至last的元素拷贝到从result开始的已分配的内存中，返回结束位置
     */
    template<typename InputIterator, typename ForwardIterator>
    ForwardIterator unchecked_uninit_copy(InputIterator first, InputIterator last, ForwardIterator result, std::true_type)
    {
        return copy(first, last, result);
    }

    template<typename InputIterator, typename ForwardIterator>
    ForwardIterator unchecked_uninit_copy(InputIterator first, InputIterator last, ForwardIterator result, std::false_type)
    {
        auto cur = result;
        try
        {
            for (; first != last; ++first, ++cur)
                construct_at(&*cur, *first);
        }
        catch (...)
        {
            for (; result != cur; --cur)
                destroy_at(&*cur);
        }
        return cur;
    }

    template<typename InputIterator, typename ForwardIterator>
    ForwardIterator uninitialized_copy(InputIterator first, InputIterator last, ForwardIterator result)
    {
        return bitstl::unchecked_uninit_copy(first, last, result,
            std::is_trivially_copy_assignable<typename bitstl::iterator_traits<ForwardIterator>::value_type>{});
    }

    /*
     * 从first至last的元素移动到从result开始的已分配的内存中，返回结束位置
     */
    template<typename InputIterator, typename ForwardIterator>
    ForwardIterator unchecked_uninit_move(InputIterator first, InputIterator last, ForwardIterator result, std::true_type)
    {
        return move(first, last, result);
    }

    template<typename InputIterator, typename ForwardIterator>
    ForwardIterator unchecked_uninit_move(InputIterator first, InputIterator last, ForwardIterator result, std::false_type)
    {
        ForwardIterator cur = result;
        try
        {
            for (; first != last; ++first, ++cur)
                construct_at(&*cur, move(*first));
        }
        catch (...)
        {
            for (; result != cur; --cur)
                destroy_at(&*cur);
        }
        return cur;
    }

    template<typename InputIterator, typename ForwardIterator>
    ForwardIterator uninitialized_move(InputIterator first, InputIterator last, ForwardIterator result)
    {
        return unchecked_uninit_move(first, last, result,
            std::is_trivially_move_assignable<typename iterator_traits<InputIterator>::value_type>{});
    }

}

#endif // !MEMORY_H




