# BitSTL

出于学习目的开发的C++标准模板库。

使用GoogleTest测试框架。

## 功能列表

`threadsafe/stack_ts.h`：线程安全的栈。无锁，使用原子类型。

`threadsafe/unordered_map_ts.h`：线程安全的哈希查找表。在bucket一级加锁。

`threadsafe/queue_ts.h`：线程安全的队列。队头队尾分别加锁。

`threadsafe/list_ts.h`：线程安全的单向链表。在节点一级加锁。

`parallel/algo_paral.h`：并发算法库。

## 参考资料

《STL源码解析》

《C++并发编程实战》第二版

[microsoft/STL](https://github.com/microsoft/STL)

[gcc-mirror/gcc](https://github.com/gcc-mirror/gcc)

[electronicarts/EASTL](https://github.com/electronicarts/EASTL)

[Alinshans/MyTinySTL](https://github.com/Alinshans/MyTinySTL)

## 补充说明

MSVC中使用`()`包围函数名，避免其被识别为宏，如`(max)`。

不同STL的vector使用的增长因子：

- MSVC 1938 - `vector` - `_Calculate_growth`，增长因子为1.5。

- GCC 13.2.0 - `stl_vector.h`  - ` _M_check_len`，增长因子为2。

- EASTL - `vector.h` - `GetNewCapacity`，增长因子为2。

- MyTinySTL - `vector.h` - `get_new_cap`，增长因子为1.5。

《STL源码解析》中对于allocator的设计是：

采用两级allocator，一级allocator直接封装malloc、free、realloc，处理要分配的内存超过128bytes的情况；二级allocator以链表形式的内存池管理小于128bytes的内存并将其统一为8的倍数，16个自由链表各自管理大小为8\~128bytes的小额区块。

