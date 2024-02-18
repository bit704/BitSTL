/*
 * 内存分配器萃取接口
 */
#ifndef ALLOCATOR_TRAITS_H
#define ALLOCATOR_TRAITS_H

#include <memory>
#include "type_traits.h"

namespace bitstl
{
    template<typename Alloc>
    struct allocator_traits
    {
    public:
        using allocator_type  = Alloc;
        // allocator类定义的类型
        using value_type      = typename Alloc::value_type;
        using size_type       = typename Alloc::size_type;
        using difference_type = typename Alloc::difference_type;
        using propagate_on_container_move_assignment = typename Alloc::propagate_on_container_move_assignment;
        using is_always_equal                        = type_identity_t<typename Alloc::is_always_equal>;
        // allocator类未定义的类型
        using pointer            = value_type*;
        using const_pointer      = const value_type*;
        using void_pointer       = void*;
        using const_void_pointer = const void*;
        using propagate_on_container_copy_assignment = false_type;
        using propagate_on_container_swap            = false_type;

    private:
        template<typename A, typename T>
        struct rebind : replace_first_arg<A, T>
        {
            static_assert(
                is_same<
                typename replace_first_arg<A, typename A::value_type>::type, 
                A>(),
                "allocator_traits<A>::rebind_alloc<A::value_type> must be A");
        };

    public:
        // 重绑定分配器到另一类型T
        template<typename T>
        using rebind_alloc  = typename rebind<Alloc, T>::type;
        template<typename T>
        using rebind_traits = allocator_traits<rebind_alloc<T>>;

        [[nodiscard]]
        static constexpr pointer allocate(Alloc& a, size_type n)
        {
            return a.allocate(n);
        }

        static constexpr void deallocate(Alloc& a, pointer p, size_type n)
        {
            a.deallocate(p, n);
        }

        // alloactor类的construct函数自C++20废弃
        template<typename T, typename... Args>
        static constexpr void construct(Alloc& a, T* p, Args&&... args)
        {
            ::new (static_cast<void*>(p)) T(std::forward<Args>(args)...);
        }

        // alloactor类的destroy函数自C++20废弃
        template< class T >
        static constexpr void destroy(Alloc& a, T* p)
        {
            p->~T();
        }

        // alloactor类的max_size函数自C++20废弃
        static constexpr size_type max_size(const Alloc& a)
            noexcept
        {
            return std::numeric_limits<size_type>::max() / sizeof(value_type);
        }

        static constexpr Alloc select_on_container_copy_construction(const Alloc& a)
        {
            return a;
        }
    };
}

#endif // !ALLOCATOR_TRAITS_H

