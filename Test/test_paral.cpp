#include "pch.h"
#include "common.h"

namespace test_paral
{
    TEST(TestSort, Test0)
    {
        std::list<int> test_list;
        for (int i = int(1e2); i >= 0; --i)
        {
            test_list.push_back(i);
        }

        auto start = steady_clock::now();

        auto comp = std::less<int>();
        auto res = quick_sort_paral(test_list, comp);
        //test_list.sort(comp);
        //auto res = test_list;

        auto end = steady_clock::now();

        ASSERT_TRUE(std::is_sorted(res.begin(), res.end(), comp));

        LOG << duration_cast<milliseconds>(end - start).count() << "(milliseconds)" << std::endl;
    }
}

