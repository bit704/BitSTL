/*
 * 类型萃取工具
 */
#ifndef TYPE_TRAITS_H
#define TYPE_TRAITS_H

// 使用内建函数实现的函数（如is_trivially_copy_assignable）使用std版本
#include <type_traits>

namespace bitstl
{
    // 整型常量
    template<typename T, T v>
    struct integral_constant
    {
        using type = integral_constant<T, v>;

        static constexpr T value = v;
        using value_type = T;
        // 重载类型转换运算符
        constexpr operator value_type()   const noexcept { return value; }
        // 重载()运算符
        constexpr value_type operator()() const noexcept { return value; }
    };

    using true_type  = integral_constant<bool, true>;
    using false_type = integral_constant<bool, false>;

    // https://en.cppreference.com/w/cpp/types/type_identity
    // 建立无类型推导上下文，避免类型推导
    template <typename T>
    struct type_identity
    {
        using type = T;
    };

    template<typename T>
    using type_identity_t = typename type_identity<T>::type;

    // 判断类型是否相等
    template <typename, typename>
    inline constexpr bool is_same_v = false;
    template <typename T>
    inline constexpr bool is_same_v<T, T> = true;

    template <typename T, typename U>
    struct is_same : integral_constant<bool, is_same_v<T, U>> {};

    // 将SomeTemplate模板第一个模板参数替换为U
    template<typename T, typename U>
    struct replace_first_arg {};

    template<
        template<typename, typename...> typename SomeTemplate, 
        typename U,
        typename T, 
        typename... Types>
    struct replace_first_arg<SomeTemplate<T, Types...>, U>
    {
        using type = SomeTemplate<U, Types...>;
    };

    // 利用SFINAE在模板元编程中检查表达式是否有效
    template<typename...> 
    using void_t = void;

    // 去掉const
    template<typename T>
    struct remove_const
    {
        typedef T type;
    };

    template<typename T>
    struct remove_const<const T>
    {
        typedef T type;
    };

    template<typename T>
    using remove_const_t = typename remove_const<T>::type;

    // 去掉volatile
    template<typename T>
    struct remove_volatile
    {
        typedef T type;
    };

    template<typename T>
    struct remove_volatile<volatile T>
    {
        typedef T type;
    };

    template<typename T>
    using remove_volatile_t = typename remove_volatile<T>::type;

    // 去掉const和volatile
    template<typename T>
    struct remove_cv
    {
        using type = T;
    };

    template<typename T>
    struct remove_cv<const T>
    {
        using type = T;
    };

    template<typename T>
    struct remove_cv<volatile T>
    {
        using type = T;
    };

    template<typename T>
    struct remove_cv<const volatile T>
    {
        using type = T;
    };

    template<typename T>
    using remove_cv_t = typename remove_cv<T>::type;

    // 添加const
    template<typename T>
    struct add_const
    {
        using type = const T;
    };

    template<typename T>
    using add_const_t = typename add_const<T>::type;

    // 添加volatile
    template<typename T>
    struct add_volatile
    {
        using type = volatile T;
    };

    template<typename T>
    using add_volatile_t = typename add_volatile<T>::type;

    // 添加const volatile
    template<typename T>
    struct add_cv
    {
        using type = const volatile T;
    };

    template<typename T>
    using add_cv_t = typename add_cv<T>::type;

    // 去掉引用
    template<typename T>
    struct remove_reference
    {
        using type = T;
    };

    template<typename T>
    struct remove_reference<T&>
    {
        using type = T;
    };

    template<typename T>
    struct remove_reference<T&&>
    {
        using type = T;
    };

    template <typename T>
    using remove_reference_t = typename remove_reference<T>::type;

    // 添加左值引用
    template<typename T, typename = void>
    struct add_lvalue_reference_helper
    {
        using type = T;
    };

    template<typename T>
    struct add_lvalue_reference_helper<T, void_t<T&>>
    {
        using type = T&;
    };

    template<typename T>
    struct add_lvalue_reference
    {
        using type = typename add_lvalue_reference_helper<T>::type;
    };

    template<typename T>
    using add_lvalue_reference_t = typename add_lvalue_reference<T>::type;

    // 添加右值引用
    template<typename T, typename = void>
    struct add_rvalue_reference_helper
    {
        using type = T;
    };

    template<typename T>
    struct add_rvalue_reference_helper<T, void_t<T&&>>
    {
        using type = T&&;
    };

    template<typename T>
    struct add_rvalue_reference
    {
        using type = typename add_rvalue_reference_helper<T>::type;
    };

    template<typename T>
    using add_rvalue_reference_t = typename add_rvalue_reference<T>::type;

    // 是否是左值引用
    template<typename>
    struct is_lvalue_reference : public false_type {};

    template<typename T>
    struct is_lvalue_reference<T&> : public true_type {};

    template<typename T>
    using is_lvalue_reference_v = is_lvalue_reference<T>::value;

    // 是否是右值引用
    template<typename>
    struct is_rvalue_reference : public false_type {};

    template<typename T>
    struct is_rvalue_reference<T&&> : public true_type {};

    template<typename T>
    using is_rvalue_reference_v = is_rvalue_reference<T>::value;

    // forward完美转发，利用引用折叠
    template<typename T>
    [[nodiscard]]
    constexpr T&& forward(remove_reference_t<T>& t) 
        noexcept
    {
        return static_cast<T&&>(t);
    }

    template<typename T>
    [[nodiscard]]
     constexpr T&& forward(remove_reference_t<T>&& t) noexcept
    {
        static_assert(!is_lvalue_reference_v<T>,
            "forward must not be used to convert an rvalue to an lvalue");
        return static_cast<T&&>(t);
    }

     // move转为右值
     template<typename T>
     [[nodiscard]]
     constexpr remove_reference_t<T>&& move(T&& t)
         noexcept
     {
         return static_cast<remove_reference_t<T>&&>(t);
     }

     // enable_if_t只有当Cond为true时才被定义
     template<bool, typename T = void>
     struct enable_if
     { };

     template<typename T>
     struct enable_if<true, T>
     {
         typedef T type;
     };

     template<bool Cond, typename T = void>
     using enable_if_t = typename enable_if<Cond, T>::type;
}

#endif // !TYPE_TRAITS_H
