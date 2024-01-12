#include "pch.h"
#include "../BitSTL/threadsafe_stack.h"

namespace test_threadsafe_stack
{
    template<typename T>
    void push_and_pop(threadsafe_stack<T>& tstk)
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

    TEST(TestConcurrency, Int)
    {
        threadsafe_stack<int> tstk;
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

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}