/*
 * 基本设置  
 */
#ifndef CONFIG_H
#define CONFIG_H

// https://devblogs.microsoft.com/oldnewthing/20220408-00/?p=106438
// 用于两层noexcept嵌套，内层的是noexcept操作符，外层是noexcept说明符
// noexcept(noexcept(...))
#define NOEXCEPT_IF(...) noexcept(__VA_ARGS__)

namespace bitstl
{
    using size_t    = unsigned long long int;
    using ptrdiff_t = long long int;
}

#endif // !CONFIG_H

