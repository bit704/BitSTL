#include "pch.h"
#include "common.h"

namespace test_paral
{
    TEST(TestSort, TestQuick_sort)
    {
        std::list<int> test_list;

        test_list.assign(500, 0);
        std::iota(test_list.begin(), test_list.end(), 1);
        auto comp = std::less<int>();

        BENCHMARK(auto res = quick_sort_paral(test_list, comp);,
            test_list.sort(comp);)

        ASSERT_TRUE(std::is_sorted(res.begin(), res.end(), comp));
    }

    TEST(TestCalc, TestAccumulate)
    {
        std::vector<double> test_vec(int(1e5), 2.2);

        double res1 = 0, res2 = 0;
        BENCHMARK(res1 = accumulate_paral(test_vec.begin(), test_vec.end(), 0.);,
            res2 = std::accumulate(test_vec.begin(), test_vec.end(), 0.);)

        ASSERT_NEAR(res1, res2, 1e-5);
    }
}

