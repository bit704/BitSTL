#include "pch.h"

#include "../BitSTL/threadsafe/stack_ts.h"
#include "../BitSTL/threadsafe/queue_ts.h"

using namespace bitstl;

namespace test_threadsafe
{
    template<typename T>
    void push_and_pop(stack_ts<T>& test_stk)
    {
        std::thread::id id = std::this_thread::get_id();
        for (int i = 0; i < 5; ++i)
        {
            test_stk.push(i);
            std::cout << "thread(" << id << ")";
            std::cout << "push:" << i << std::endl;
        }
        for (int i = 0; i < 5; ++i)
        {
            std::shared_ptr<T> p = test_stk.pop();
            std::cout << "thread(" << id << ")";
            std::cout << "pop:" << *p << std::endl;
        }
        return;
    }

    TEST(TestStack, Test0)
    {
        stack_ts<int> test_stk;
        std::vector<std::thread> vt;
        vt.resize(2);
        for (int i = 0; i < vt.size(); ++i)
        {
            vt[i] = std::thread(push_and_pop<int>, std::ref(test_stk));
        }
        for (int i = 0; i < vt.size(); ++i)
        {
            vt[i].join();
        }
        ASSERT_TRUE(test_stk.empty());
    }

    queue_ts<int> test_que;

    void data_preparation(int num)
    {
        while (num--)
        {
            test_que.push(num);
        }
    }

    void data_process(int num)
    {
        int count = 0;
        while (count < num)
        {
            int t;
            test_que.wait_and_pop(t);
            std::cout << "process " << t << std::endl;
            ++count;
        }
    }

    TEST(TestQueue, Test0)
    {
        std::thread t1 = std::thread(data_process, 10);
        std::thread t2 = std::thread(data_preparation, 10);
        t1.join();
        t2.detach();
        ASSERT_TRUE(test_que.empty());
    }
}