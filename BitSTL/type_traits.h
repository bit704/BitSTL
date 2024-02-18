/*
 * 类型萃取工具
 */
#ifndef TYPE_TRAITS_H
#define TYPE_TRAITS_H

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

    
    template<typename T, typename U>
    struct replace_first_arg {};

    // 将SomeTemplate模板第一个模板参数替换为U
    template<
        template<typename, typename...> typename SomeTemplate, 
        typename U,
        typename T, 
        typename... Types>
    struct replace_first_arg<SomeTemplate<T, Types...>, U>
    {
        using type = SomeTemplate<U, Types...>;
    };

}

#endif // !TYPE_TRAITS_H
