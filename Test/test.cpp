#include "pch.h"
#include "common.h"

namespace test_type_traits
{
    template<typename T>
    T add(T a, type_identity_t<T> b) { return a + b; }

    template<typename T>
    class Foo {};

    TEST(Test_type_identity_t, Test0)
    {
        ASSERT_EQ(add(1.f, 2), 1.f + 2);
    }

    TEST(Test_is_same, Test0)
    {
        ASSERT_TRUE((is_same_v<int, int>));
        ASSERT_FALSE((is_same_v<int, float>));
        ASSERT_FALSE((is_same<float, Foo<float>>()));
        ASSERT_FALSE((is_same<float, double>{}));
    }

    TEST(Test_replace_first_arg, Test0)
    {
        ASSERT_TRUE((is_same_v<
            Foo<int>,
            typename replace_first_arg<Foo<float>, int>::type>));
    }
}

namespace test_vector
{
    class Foo
    {
    public:
        int x;
        Foo() { x = -1; }
        explicit Foo(int _x) : x(_x + 1) {}
        bool operator==(const Foo& other)
            const
        {
            return x == other.x;
        }
        auto operator<=>(const Foo& other)
            const
        {
            return x <=> other.x;
        }
    };

    template <typename T>
    T sum(T* arr, size_t size) 
    {
        remove_const_t<T> res = remove_const_t<T>();
        for (size_t i = 0; i < size; ++i)
            res += arr[i];
        return res;
    }

    TEST(Test_ctor, Test0)
    {
        vector<int> v1();

        vector<int> v2(10);
        ASSERT_EQ(v2[5], 0);

        vector<int> v3(200, 3);
        ASSERT_EQ(v3[127], 3);

        int a[3] = { 4, 5, 6 };
        ASSERT_TRUE(is_input_iterator<int*>);
        vector<int> v4(a, a+3);
        ASSERT_EQ(v4[2], 6);

        vector<int> v5(v4);
        ASSERT_EQ(v5[2], v4[2]);

        vector<int> v6(move(v5));
        ASSERT_EQ(v6[2], v4[2]);

        vector<int> v7{ 7, 8, 9 };
        ASSERT_EQ(v7[2], 9);
    }

    TEST(Test_ctor, Test1)
    {
        vector<Foo> v1();

        vector<Foo> v2(10);
        ASSERT_EQ(v2[5].x, -1);

        vector<Foo> v3(200, Foo(3));
        ASSERT_EQ(v3[127].x, 4);

        Foo a[3] = { Foo(4), Foo(5), Foo(6) };
        ASSERT_TRUE(is_input_iterator<Foo*>);
        vector<Foo> v4(a, a + 3);
        ASSERT_EQ(v4[2].x, 7);

        vector<Foo> v5(v4);
        ASSERT_EQ(v5[2], v4[2]);

        vector<Foo> v6(move(v5));
        ASSERT_EQ(v6[2], v4[2]);

        vector<Foo> v7{ Foo(7), Foo(8), Foo(9) };
        ASSERT_EQ(v7[2].x, 10);
    }

    TEST(Test_data_access, Test0)
    {
        vector<int> v{1, 2, 3};
        const vector<int> cv{ 4, 5, 6 };
        ASSERT_EQ( *v.begin(),   v[0]);
        ASSERT_EQ(*cv.begin(),  cv[0]);
        ASSERT_EQ(*cv.cbegin(), cv[0]);
        ASSERT_EQ( *(v.end()  - 1),  v[2]);
        ASSERT_EQ(*(cv.end()  - 1), cv[2]);
        ASSERT_EQ(*(cv.cend() - 1), cv[2]);
        ASSERT_EQ( *v.rbegin(),   v[2]);
        ASSERT_EQ(*cv.rbegin(),  cv[2]);
        ASSERT_EQ(*cv.crbegin(), cv[2]);
        ASSERT_EQ( *(v.rend()  - 1),  v[0]);
        ASSERT_EQ(*(cv.rend()  - 1), cv[0]);
        ASSERT_EQ(*(cv.crend() - 1), cv[0]);
    }

    TEST(Test_data_access, Test1)
    {
        vector<int> v{ 1, 2, 3 };
        const vector<int> cv{ 4, 5, 6 };
        ASSERT_EQ( v[0],  v.at(0));
        ASSERT_EQ(cv[0], cv.at(0));
        ASSERT_EQ( v.front(),  v.at(0));
        ASSERT_EQ(cv.front(), cv.at(0));
        ASSERT_EQ( v.back(),   v.at(2));
        ASSERT_EQ(cv.back(),  cv.at(2));

    }

    TEST(Test_data_access, Test2)
    {
        vector<int> v{ 1, 2, 3 };
        const vector<int> cv{ 4, 5, 6 };
        ASSERT_EQ(sum( v.data(),  v.size()),  6);
        ASSERT_EQ(sum(cv.data(), cv.size()), 15);
    }

    TEST(Test_swap, Test0)
    {
        vector<int> v1{ 1, 2, 3 };
        vector<int> v2{ 4, 5, 6 };
        v1.swap(v2);
        ASSERT_EQ(v1[0], 4);
        ASSERT_EQ(v2[0], 1);
    }

    TEST(Test_swap, Test1)
    {
        vector<int> v1{ 1, 2, 3 };
        vector<int> v2{ 4, 5, 6 };
        swap(v1, v2);
        ASSERT_EQ(v1[0], 4);
        ASSERT_EQ(v2[0], 1);
    }

    TEST(Test_capacity, Test0)
    {
        vector<int> v1{};
        ASSERT_TRUE(v1.empty());
        ASSERT_EQ(v1.size(), 0);
        ASSERT_EQ(v1.max_size(), static_cast<size_t>(-1) / sizeof(int));
        ASSERT_EQ(allocator<int>(), v1.get_allocator());
    }

    TEST(Test_capacity, Test1)
    {
        vector<int> v2{ 1, 2, 3 };
        ASSERT_EQ(v2.capacity(), 3);
        v2.reserve(8);
        ASSERT_EQ(v2.capacity(), 8);
        v2.shrink_to_fit();
        ASSERT_EQ(v2.capacity(), 3);
    }

    TEST(Test_assign, Test0)
    {
        vector<int> v1{ 1, 2 };
        vector<int> v2{ 1, 2, 3, 4 };
        v1 = v2;
        ASSERT_EQ(v1.size(), 4);
        ASSERT_EQ(v1.capacity(), 4);
    }

    TEST(Test_assign, Test1)
    {
        vector<int> v1{ 1, 2, 3, 4 };
        vector<int> v2{ 1, 2 };
        v1 = v2;
        ASSERT_EQ(v1.size(), 2);
        ASSERT_EQ(v1.capacity(), 4);
    }

    TEST(Test_assign, Test2)
    {
        vector<int> v1{ 1, 2 };
        vector<int> v2{ 1, 2, 3, 4 };
        v1.reserve(6);
        v1 = v2;
        ASSERT_EQ(v1.size(), 4);
        ASSERT_EQ(v1.capacity(), 4);
    }

    TEST(Test_assign, Test3)
    {
        vector<int> v1{ 1, 2 };
        vector<int> v2{ 1, 2, 3, 4 };
        v1 = move(v2);
        ASSERT_EQ(v1.size(), 4);
        ASSERT_EQ(v1.capacity(), 4);
        ASSERT_EQ(v2.size(), 0);
    }

    TEST(Test_assign, Test4)
    {
        vector<int> v1{ 1, 2 };
        v1 = { 1, 2, 3, 4 };
        ASSERT_EQ(v1.size(), 4);
        ASSERT_EQ(v1.capacity(), 4);
    }
}

namespace test_delegate
{
    class Foo
    {      
    public:
        int x = 0;

        Foo() {}
        ~Foo() {}

        int add(int a, int b)
        {
            return a + b;
        }

        void store_add(int a, int b)
        {
            x = a + b;
        }
    };
    
    int add(int a, int b)
    {
        return a + b;
    }

    TEST(Test_delegate_twoparam, Test0)
    {
        Foo test;
        auto delegate1 = 
            delegate_twoparam<int, int, int>
            ::from_class_func<Foo, &Foo::add>(&test);
        ASSERT_EQ(delegate1(1, 3), 4);
        ASSERT_EQ(delegate1.execute(2, 2), 4);
        ASSERT_TRUE(delegate1.isbound());
        delegate1.unbind();
        ASSERT_FALSE(delegate1.isbound());
        delegate1.bind_func<&add>();
        ASSERT_EQ(delegate1(1, 7), 8);

        auto delegate2 =
            delegate_twoparam<int, int, int>
            ::from_func<&add>();
        ASSERT_EQ(delegate2(2, 4), 6);
        ASSERT_EQ(delegate2.execute(2, 4), 6);
        ASSERT_TRUE(delegate2.isbound());
        delegate2.unbind();
        ASSERT_FALSE(delegate2.isbound());
        delegate2.bind_class_func<Foo, &Foo::add>(&test);
        ASSERT_EQ(delegate2(1, 5), 6);
        ASSERT_FALSE(delegate2.execute_ifbound(2, 3));

        auto delegate3 =
            delegate_twoparam<void, int, int>
            ::from_class_func<Foo, &Foo::store_add>(&test);
        ASSERT_TRUE(delegate3.execute_ifbound(2, 3));
        ASSERT_TRUE(test.x, 5);        
    }
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}