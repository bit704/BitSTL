#include "pch.h"
#include "../BitSTL/threadsafe/stack_ts.h"

namespace test_threadsafe
{
    template<typename T>
    void push_and_pop(stack_ts<T>& tstk)
    {
        std::thread::id id = std::this_thread::get_id();
        for (int i = 0; i < 5; ++i)
        {
            tstk.push(i);
            std::cout << "thread(" << id << ")";
            std::cout << "push:" << i << std::endl;
        }
        for (int i = 0; i < 5; ++i)
        {
            std::shared_ptr<T> p = tstk.pop();
            std::cout << "thread(" << id << ")";
            std::cout << "pop:" << *p << std::endl;
        }
        return;
    }

    TEST(TestStack, Int)
    {
        stack_ts<int> tstk;
        std::vector<std::thread> vt;
        vt.resize(2);
        for (int i = 0; i < vt.size(); ++i)
        {
            vt[i] = std::thread(push_and_pop<int>, std::ref(tstk));
        }
        for (int i = 0; i < vt.size(); ++i)
        {
            vt[i].join();
        }
        ASSERT_TRUE(tstk.empty());
    }
}