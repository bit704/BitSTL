/*
 * 内存分配器
 */
#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include "config.h"
#include "type_traits.h"

namespace bitstl
{
    template<typename T>
    class allocator
    {
    public:
        using value_type      = T;
        using size_type       = size_t;
        using difference_type = ptrdiff_t;

        // 当容器被移动赋值时，源容器的分配器将被复制到目标容器
        using propagate_on_container_move_assignment = true_type;
        using is_always_equal                        = true_type;

        constexpr allocator() noexcept {}

        constexpr allocator(const allocator&) noexcept {}

        template<typename T>
        constexpr allocator(const allocator<T>&) noexcept {}

        constexpr ~allocator() noexcept {}

# define BITSTL_OPERATOR_NEW    ::operator new
# define BITSTL_OPERATOR_DELETE ::operator delete

        // C++标准允许n为0，但未规定其对应的返回值
        [[nodiscard]] 
        constexpr T* allocate(size_type n)
        {
            // 不完整类型（前向声明的类或未定义的类）大小为0
            static_assert(sizeof(T) != 0, "cannot allocate incomplete types");

            // 无法分配
            if (n > (size_t(-1) / sizeof(T)))
                throw std::bad_array_new_length();

            // C++17提供__STDCPP_DEFAULT_NEW_ALIGNMENT__，超过阈值时可强行对齐
            if (alignof(T) > __STDCPP_DEFAULT_NEW_ALIGNMENT__)
            {
                std::align_val_t align_val = std::align_val_t(alignof(T));
                return static_cast<T*>(BITSTL_OPERATOR_NEW(n * sizeof(T), align_val));
            }

            return static_cast<T*>(BITSTL_OPERATOR_NEW(n * sizeof(T)));
        }

        // p不允许为空指针
        constexpr void deallocate(T* p, size_type n)
        {
            if (alignof(T) > __STDCPP_DEFAULT_NEW_ALIGNMENT__)
            {
                BITSTL_OPERATOR_DELETE(p, n * sizeof(T), std::align_val_t(alignof(T)));
                return;
            }

            BITSTL_OPERATOR_DELETE(p, n * sizeof(T));
        }

#undef BITSTL_OPERATOR_DELETE
#undef BITSTL_OPERATOR_NEW

        template<typename U>
        friend constexpr bool operator==(const allocator&, const allocator<U>&)
            noexcept
        { 
            return true; 
        }

            template<typename U>
        friend constexpr bool operator!=(const allocator&, const allocator<U>&)
            noexcept
        {
            return false;
        }
    };
}

#endif // !ALLOCATOR_H

