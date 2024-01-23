# BitSTL

C++标准模板库。

使用GoogleTest测试框架。

功能列表：

`threadsafe/stack_ts.h`：线程安全的栈。无锁，使用原子类型。

`threadsafe/unordered_map_ts.h`：线程安全的哈希查找表。在bucket一级加锁。

`threadsafe/queue_ts.h`：线程安全的队列。队头队尾分别加锁。

`threadsafe/list_ts.h`：线程安全的单向链表。在节点一级加锁。

`parallel/algo_paral.h`：并发算法库。

参考资料：

《STL源码解析》

《C++并发编程实战》第二版

[Alinshans/MyTinySTL](https://github.com/Alinshans/MyTinySTL)

[electronicarts/EASTL](https://github.com/electronicarts/EASTL)
