# BitSTL

C++标准模板库，使用GoogleTest测试框架。

## 功能

`config.h`：C++基本设置。

`type_traits.h`：类型萃取工具库。

`allocator.h`：内存分配器。

`allocator_traits.h`：内存分配器萃取接口。

`iterator.h`：迭代器。

`memory.h`：动态内存管理库。

`algorithm.h`：算法库。

`vector.h`：动态连续数组。

`delegate.h`：委托。

`threadsafe/stack_ts.h`：线程安全的栈。无锁，使用原子类型。

`threadsafe/unordered_map_ts.h`：线程安全的哈希查找表。在bucket一级加锁。

`threadsafe/queue_ts.h`：线程安全的队列。队头队尾分别加锁。

`threadsafe/list_ts.h`：线程安全的单向链表。在节点一级加锁。

`parallel/algo_paral.h`：并发算法库。

## 笔记

1. 参考资料：

   [cppreference](https://en.cppreference.com/w/)

   《STL源码解析》

   《C++并发编程实战》第二版

   [microsoft/STL](https://github.com/microsoft/STL) ——参考版本MSVC 1938（14.38.33130）

   [gcc-mirror/gcc](https://github.com/gcc-mirror/gcc)（libstdc++-v3/include）——参考版本GCC 13.2.0

   [electronicarts/EASTL](https://github.com/electronicarts/EASTL)

   [Alinshans/MyTinySTL](https://github.com/Alinshans/MyTinySTL)

2. MSVC中使用`()`包围函数名，避免其被识别为宏，如`(max)`。另一种方法是使用`#undef`。

3. MSVC对于模板的编译支持较差，往往不能正确报错，GCC表现更好。

4. 不同STL均使用`__cplusplus`识别C++标准版本（MSVC嵌套了多层宏）。

   > [Replacing text macros - cppreference.com](https://en.cppreference.com/w/cpp/preprocessor/replace#Predefined_macros)

5. 使用`_MSC_VER`、`__GNUC__`、`__clang__`（`__clang_major__`、`__clang_minor__`、`__clang_patchlevel__`）可识别当前编译器类型及版本。

6. GCC的 `__builtin_` 前缀用于命名编译器提供的**内建函数**（不是C++ 标准的一部分），如`__builtin_popcount` 计算整数中1的个数、`__builtin_expect`提供分支预测的指示给编译器、`__builtin_addressof`绕过`operator&`重载获取地址。

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

7. 对于未使用的变量、函数，GCC可使用`__attribute__((__unused__))`来消除未使用警告。MSVC可使用`(void)`消除未使用变量的警告，但对于未使用函数没有办法。

8. `allocator`、`allocator_traits`参考设计：

   - 在C++20中，`allocator`的`address`、`max_size`、`construct`、`destroy`被废弃，仅保留`allocate`和`deallocate`。为了支持constexpr容器，`allocator`、`allocator_traits`的成员函数均转为**constexpr**实现。

     > [c++ - Why are are std::allocator's construct and destroy functions deprecated in c++17? - Stack Overflow](https://stackoverflow.com/questions/39414610/why-are-are-stdallocators-construct-and-destroy-functions-deprecated-in-c17)
     >
     > [如何评价新并入 C++20 的 constexpr allocation? - 知乎 (zhihu.com)](https://www.zhihu.com/question/336059175)

   - MSVC、GCC、MyTinySTL的`allocator`直接封装`::operator new`、`::operator delete`。

   - EASTL的`allocator`直接封装`malloc`、`free`，不是模板，类似`std::allocator<char>`。

   - 《STL源码解析》中采用两级`allocator`，一级`allocator`直接封装`malloc`、`free`、`realloc`，处理要分配的内存超过128bytes的情况；二级`allocator`以链表形式的**内存池**管理小于128bytes的内存并将其统一为8的倍数，16个自由链表各自管理大小为8\~128bytes的小额区块。

   - C++17引入了`std::pmr`命名空间，可以逐对象指定内存资源类型。

9. `vector`参考设计：

   - MSVC - `vector` - `_Calculate_growth`，增长因子为1.5。

   - GCC - `stl_vector.h`  - ` _M_check_len`，增长因子为2。GCC中`vector`protected继承于`_Vector_base`，将且仅将内存操作放入其中。

   - EASTL - `vector.h` - `GetNewCapacity`，增长因子为2。

   - MyTinySTL - `vector.h` - `get_new_cap`，增长因子为1.5。

   - 《STL源码解析》图4-2使用增长因子为2。

   - `emplace_back`于C++11引入，于C++17新增了返回新插入元素的迭代器的功能。

10. 使用SFINAE的两种写法：

    ```c++
    // 写法1：使用非类型模板参数
    template<
        typename InputIterator,
        typename enable_if_t<is_input_iterator<InputIterator>, int> = 0
    >
    // 写法2：使用类型模板参数
    template<
        typename InputIterator,
        typename = enable_if_t<is_input_iterator<InputIterator>>
    >
    ```

    > void不可作为非类型模板参数。
    >
    > 允许以下类型作为非类型模板参数：
    >
    > 1. 整型或枚举
    > 
    > 2. 指向对象或函数的指针
    > 
    > 3. lvalue引用类型（C++11）
    > 
    > 4. `std::nullptr_t`类型（C++11）
    > 
    > 5. 指向成员的指针（C++11）
    > 
    > 6. 浮点和类类型（C++20）

11. Java（JDK11.0.8）的`Arrays.sort()`采用如下排序策略：
    - 数组长度达到286，且不存在较多连续相等元素（小于33），且高度结构化（划分的单调递增/递减的块数量小于67），采用类**TimSort**进行排序。
    - 当数组长度小于47，当前排序的部分位于数组的最左侧时使用无sentinel的**插入排序**，否则使用带sentinel的**双插入排序**。
    - 其他情况采用**双轴快排**。双轴快排时若中间区域大于数组长度的$4/7$，将中间区域分为$[p1,p1]$、$(p1,p2)$、$[p2,p2]$，只让$(p1,p2)$参与下一轮双轴快排。双轴快排时每轮选取五个备选轴（步长为数组长度的$1/7$），只取第二个轴和第四个轴作为排序轴，若五个备选轴中有相等元素，则可以认为数组中存在较多相等元素，此时取第三个轴进行**单轴分治**。

