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
        ASSERT_TRUE((is_same_v<int, int>));
        ASSERT_FALSE((is_same_v<int, float>));
    }

    template<typename T>
    class Foo {};

    TEST(Test_replace_first_arg, Test0)
    {
        ASSERT_TRUE((is_same_v<
            Foo<int>,
            typename replace_first_arg<Foo<float>, int>::type>));
    }
}

namespace test_vector
{
    TEST(Test_ctor, Test0)
    {
        vector<int> v1(10);
        ASSERT_EQ(v1[5], 0);
        vector<int> v2(200, 3);
        ASSERT_EQ(v2[127], 3);
    }

    TEST(Test_ctor, Test1)
    {
        class Foo
        {
        public:
            int x;
            Foo() = default;
            Foo(int _x) : x(_x + 1) {}
        };
        vector<Foo> v1(10);
        ASSERT_EQ(v1[5].x, 0);
        vector<Foo> v2(200, 3);
        ASSERT_EQ(v2[127].x, 4);
    }
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}