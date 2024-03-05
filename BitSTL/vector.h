/*
 * 动态连续数组 
 */
#ifndef VECTOR_H
#define VECTOR_H

#include "allocator.h"
#include "allocator_traits.h"
#include "iterator.h"

namespace bitstl
{
    // vector protected继承自 vector_base
    // vector_base仅进行内存的分配和释放，vector进行对象的构造和析构，解耦，便于处理异常
    // 对没有数据成员的对象alloc实现EBO（空基类优化）
    template<typename T, typename Alloc>
    struct vector_base
    {
        using allocator_type = Alloc;
        using allocator_traits_type = allocator_traits<allocator_type>;
        using pointer  = typename allocator_traits_type::pointer;

        pointer start;          // 头元素
        pointer finish;         // 尾元素
        pointer end_of_storage; // 储存空间尾部
        allocator_type alloc; // 内存分配器萃取接口
        
        constexpr pointer allocate(size_t n)
        {
            if (n)
                return allocator_traits_type::allocate(alloc, n);
            else
                return pointer();
        }

        constexpr void deallocate(pointer p, size_t n)
        {
            if (p)
                allocator_traits_type::deallocate(alloc, p, n);
        }        
    };

    template<typename T, typename Alloc = allocator<T>>
    class vector : protected vector_base<T, Alloc>
    {
        static_assert(is_same_v<typename remove_cv<T>::type, T>,
            "vector must have a non-const, non-volatile value_type");

        static_assert(is_same_v<typename Alloc::value_type, T>,
            "vector must have the same value_type as its allocator");

    private:
        using Alloc_traits = allocator_traits<Alloc>;

    public:
        /*
         * 成员类型 
         */
        using value_type      = T;
        using allocator_type  = Alloc;
        using size_type       = size_t;
        using difference_type = ptrdiff_t;
        using reference       = value_type&;
        using const_reference = const value_type&;
        using pointer         = typename Alloc_traits::pointer;
        using const_pointer   = typename Alloc_traits::const_pointer;
        // 迭代器
        using iterator        = bitstl::iterator_adpater<pointer, vector>;
        using const_iterator  = bitstl::iterator_adpater<const_pointer, vector>;
        using reverse_iterator       = bitstl::reverse_iterator<iterator>;
        using const_reverse_iterator = bitstl::reverse_iterator<const_iterator>;
        
        /*
         * 构造函数 
         */
        constexpr vector() noexcept = default;

        explicit constexpr vector(size_type count)
        {
            fill(count, value_type());
        }

        constexpr vector(size_type count, const value_type& value) 
        {
            fill(count, value);
        }


        /*
         * 元素访问 
         */
        reference operator[](size_type n)
        {
            return *(this->start + n);
        }

        const_reference operator[](size_type n) 
            const
        {
            return *(this->start + n);
        }

    private:
        void try_allocate(size_type count)
        {
            try
            {
                this->start  = this->allocate(count);
                this->finish = this->start + count;
                this->end_of_storage = this->start + count;
            }
            catch (...)
            {
                this->start = nullptr;
                this->finish = nullptr;
                this->end_of_storage = nullptr;
                throw;
            }
        }

        void fill(size_type count, const value_type& value)
        {
            try_allocate(count);
            bitstl::uninitialized_fill_n(this->start, count, value);
        }
    };
}

#endif // !VECTOR_H

