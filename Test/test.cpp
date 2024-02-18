#include "pch.h"
#include "common.h"

namespace test_type_traits
{
    template<typename T>
    T add(T a, type_identity_t<T> b) { return a + b; }

    TEST(Test_type_identity_t, Test0)
    {
        ASSERT_EQ(add(1.f, 2), 1.f + 2);
    }

    TEST(Test_is_same, Test0)
    {
        ASSERT_TRUE((is_same<int, int>()));
        ASSERT_FALSE((is_same<int, float>()));
    }

    template<typename T>
    class Foo {};

    TEST(Test_replace_first_arg, Test0)
    {
        ASSERT_TRUE((is_same<
            Foo<int>,
            typename replace_first_arg<Foo<float>, int>::type>()));

    }
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}