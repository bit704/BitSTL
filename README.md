# BitSTL

出于学习目的开发的C++标准模板库，基于C++20。

使用GoogleTest测试框架。

## 功能

`threadsafe/stack_ts.h`：线程安全的栈。无锁，使用原子类型。

`threadsafe/unordered_map_ts.h`：线程安全的哈希查找表。在bucket一级加锁。

`threadsafe/queue_ts.h`：线程安全的队列。队头队尾分别加锁。

`threadsafe/list_ts.h`：线程安全的单向链表。在节点一级加锁。

`parallel/algo_paral.h`：并发算法库。

## 笔记

1. 参考以下资料：

   《STL源码解析》

   《C++并发编程实战》第二版

   [microsoft/STL](https://github.com/microsoft/STL) ——参考版本MSVC 1938（14.38.33130）

   [gcc-mirror/gcc](https://github.com/gcc-mirror/gcc)（libstdc++-v3/include）——参考版本GCC 13.2.0

   [electronicarts/EASTL](https://github.com/electronicarts/EASTL)

   [Alinshans/MyTinySTL](https://github.com/Alinshans/MyTinySTL)

2. MSVC中使用`()`包围函数名，避免其被识别为宏，如`(max)`。

3. 不同STL均使用`__cplusplus`识别C++标准版本（MSVC嵌套了多层宏）。

   > [Replacing text macros - cppreference.com](https://en.cppreference.com/w/cpp/preprocessor/replace#Predefined_macros)

4. 使用`_MSC_VER`、`__GNUC__`、`__clang__`（`__clang_major__`、`__clang_minor__`、`__clang_patchlevel__`）可识别当前编译器类型及版本。

5. GCC的 `__builtin_` 前缀用于命名编译器提供的**内建函数**（不是C++ 标准的一部分），如`__builtin_popcount` 计算整数中1的个数、`__builtin_expect`提供分支预测的指示给编译器、`__builtin_addressof`绕过`operator&`重载获取地址。

   MSVC中有类似的内建函数，如GCC的`__builtin_popcount`在 MSVC 中对应 `__popcnt` 。

   如果自行实现，需要使用**内联汇编**：

   ```c++
   #include <iostream>
   
   #ifdef __GNUC__
   int popcnt(int value)
   {
       int result;
       __asm__(
           "popcnt %1, %0"
           : "=r"(result)
           : "r"(value));
       return result;
   }
   #elif defined(_MSC_VER)
   // MSVC不支持x64内联汇编，需要x86编译
   unsigned int popcnt(unsigned int value)
   {
       __asm
       {
           mov eax, value
           popcnt eax, eax
       }
   }
   #endif
   
   int main()
   {
       int x = 10; // 二进制表示为1010，有两个1
       std::cout << popcnt(x) << std::endl;
       return 0;
   }
   ```

6. `allocator`、`allocator_traits`参考设计：

   - 在C++20中，`allocator`的`address`、`max_size`、`construct`、`destroy`被废弃，仅保留`allocate`和`deallocate`。为了支持constexpr容器，`allocator`、`allocator_traits`的成员函数均转为**constexpr**实现。

     > [c++ - Why are are std::allocator's construct and destroy functions deprecated in c++17? - Stack Overflow](https://stackoverflow.com/questions/39414610/why-are-are-stdallocators-construct-and-destroy-functions-deprecated-in-c17)
     >
     > [如何评价新并入 C++20 的 constexpr allocation? - 知乎 (zhihu.com)](https://www.zhihu.com/question/336059175)

   - MSVC、GCC、MyTinySTL的`allocator`直接封装`::operator new`、`::operator delete`。

   - EASTL的`allocator`直接封装`malloc`、`free`，不是模板，类似`std::allocator<char>`。

   - 《STL源码解析》中采用两级`allocator`，一级`allocator`直接封装`malloc`、`free`、`realloc`，处理要分配的内存超过128bytes的情况；二级`allocator`以链表形式的**内存池**管理小于128bytes的内存并将其统一为8的倍数，16个自由链表各自管理大小为8\~128bytes的小额区块。

   - C++17引入了`std::pmr`命名空间，可以逐对象而不是逐类型指定内存分配器类型，目前BitSTL不支持。

7. `vector`参考设计：

   - MSVC - `vector` - `_Calculate_growth`，增长因子为1.5。
   - GCC - `stl_vector.h`  - ` _M_check_len`，增长因子为2。GCC中`vector`protected继承于`_Vector_base`，将且仅将内存操作放入其中。
   - EASTL - `vector.h` - `GetNewCapacity`，增长因子为2。
   - MyTinySTL - `vector.h` - `get_new_cap`，增长因子为1.5。
   - 《STL源码解析》图4-2使用增长因子为2。
