#include "pch.h"
#include "common.h"

namespace test_paral
{
    TEST(TestSort, TestQuick_sort)
    {
        std::vector<int> v(int(1e4), 0);
        std::iota(v.begin(), v.end(), 1);
        auto rd = std::random_device{};
        auto rng = std::default_random_engine{ rd() };
        std::shuffle(v.begin(), v.end(), rng);

        std::list<int> test_list;
        test_list.assign(v.begin(), v.end());

        auto comp = std::greater<int>();

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

