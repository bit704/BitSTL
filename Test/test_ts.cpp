#include "pch.h"
#include "common.h"

namespace test_threadsafe
{
    stack_ts<int> test_stk;

    TEST(TestStack, Test0)
    {
        auto stack_op = []
        {
            std::thread::id id = std::this_thread::get_id();
            for (int i = 0; i < 5; ++i)
            {
                test_stk.push(i);
#ifdef DEBUGGING
                DEBUG << "thread(" << id << ")";
                DEBUG << "push:" << i << std::endl;
#endif
            }
            for (int i = 0; i < 5; ++i)
            {
                auto p = test_stk.pop();
#ifdef DEBUGGING
                DEBUG << "thread(" << id << ")";
                DEBUG << "pop:" << *p << std::endl;
#endif
            }
            return;
        };

        std::vector<std::thread> vt;
        vt.resize(3);
        for (int i = 0; i < vt.size(); ++i)
        {
            vt[i] = std::thread(stack_op);
        }
        for (int i = 0; i < vt.size(); ++i)
        {
            vt[i].join();
        }
        ASSERT_TRUE(test_stk.empty());
    }

    queue_ts<int> test_que;

    TEST(TestQueue, Test0)
    {
        std::thread t1 = std::thread([](int num)
            {
                while (num--)
                    test_que.push(num);
            }, 10);

        std::thread t2 = std::thread([](int num)
            {
                int count = 0;
                while (count < num)
                {
                    int t;
                    test_que.wait_and_pop(t);
#ifdef DEBUGGING
                    DEBUG << "process " << t << std::endl;
#endif
                    ++count;
                }
            }, 10);

        t1.detach();
        t2.join();

        ASSERT_TRUE(test_que.empty());
    }

    unordered_map_ts<int, double> test_ump;

    TEST(TestUnordered_map, Test0)
    {
        
    }

    TEST(TestUnordered_map, Test1)
    {

    }
}