#include "pch.h"
#include "common.h"

namespace test_threadsafe
{
    TEST(TestStack, Test0)
    {
        stack_ts<int> test_stk;
        int num = 10;

        auto stack_op = [&]
        {
            std::thread::id id = std::this_thread::get_id();
            for (int i = 0; i < num; ++i)
            {
                test_stk.push(i);
#ifdef DEBUGGING
                DEBUG << "thread(" << id << ")";
                DEBUG << "push:" << i << std::endl;
#endif
            }
            for (int i = 0; i < num; ++i)
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

    TEST(TestQueue, Test0)
    {
        queue_ts<int> test_que;
        int num = 10;

        std::thread t1 = std::thread([&](int num)
            {
                while (num--)
                    test_que.push(num);
            }, num);

        std::thread t2 = std::thread([&](int num)
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
            }, num);

        t1.detach();
        t2.join();

        ASSERT_TRUE(test_que.empty());
    }

    TEST(TestUnordered_map, Test0)
    {
        unordered_map_ts<int, double> test_ump;

        int tot_count = 200;
        int remove_count = 0;
        int actual_remove_count = 0;

        std::vector<int> nums(tot_count, 0);
        std::iota(nums.begin(), nums.end(), 0);

        auto rd = std::random_device{};
        auto rng = std::default_random_engine{ rd() };

        // t1和t2按不同随机序列进行插入和删除，因此remove_count有几率不等于tot_count
        std::thread t1 = std::thread([&]
            {
                auto index = nums;
                std::shuffle(std::begin(index), std::end(index), rng);
                for (int i = 0; i < index.size(); ++i)
                {
                    test_ump.add_or_update_value(index[i], std::sqrt(index[i]));
                }
            });

        std::thread t2 = std::thread([&]
            {
                auto index = nums;
                std::shuffle(std::begin(index), std::end(index), rng);
                for (int i = 0; i < index.size(); ++i)
                {
                    if (test_ump.remove_value(index[i]))
                    {
                        remove_count += 1;
#ifdef DEBUGGING
                        DEBUG << "remove " << index[i] << std::endl;
#endif
                    }
                }
            });

        t1.join();
        t2.join();

        for (int i = 0; i < tot_count; ++i)
        {
            if (test_ump.get_value(i, -1) == -1)
                actual_remove_count += 1;
        }

#ifdef DEBUGGING
        DEBUG << "remove_count: " << remove_count << std::endl;
#endif
        ASSERT_EQ(remove_count, actual_remove_count);
    }
}