#include "pch.h"
#include "common.h"

namespace test_paral
{
    TEST(TestSort, Test0)
    {
        std::list<int> test_list;

        test_list.assign(500, 0);
        std::iota(test_list.begin(), test_list.end(), 1);
        auto comp = std::less<int>();

        BENCHMARK(auto res = quick_sort_paral(test_list, comp);,
            test_list.sort(comp););

        ASSERT_TRUE(std::is_sorted(res.begin(), res.end(), comp));
    }
}

