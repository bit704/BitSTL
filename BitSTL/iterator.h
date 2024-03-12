/*
 * 迭代器 
 */
#ifndef ITERATOR_H
#define ITERATOR_H

#include "config.h"
#include "type_traits.h"

namespace bitstl
{
    // 输入迭代器
    struct input_iterator_tag {};
    // 输出迭代器
    struct output_iterator_tag {};
    // 前向迭代器
    struct forward_iterator_tag : public input_iterator_tag {};
    // 双向迭代器
    struct bidirectional_iterator_tag : public forward_iterator_tag {};
    // 随机访问迭代器
    struct random_access_iterator_tag : public bidirectional_iterator_tag {};
    // 连续迭代器（内存中连续存储）
    struct contiguous_iterator_tag : public random_access_iterator_tag {};

    /*
     *  迭代器萃取接口 
     */
    template<typename Iterator, typename = void>
    struct iterator_traits_helper {};

    template<typename Iterator>
    struct iterator_traits_helper<Iterator, 
        void_t<
        typename Iterator::iterator_category,
        typename Iterator::value_type,
        typename Iterator::difference_type,
        typename Iterator::pointer,
        typename Iterator::reference>>
    {
        using iterator_concept  = typename Iterator::iterator_concept;
        using iterator_category = typename Iterator::iterator_category;
        using value_type        = typename Iterator::value_type;
        using difference_type   = typename Iterator::difference_type;
        using pointer           = typename Iterator::pointer;
        using reference         = typename Iterator::reference;
    };

    template<typename Iterator>
    struct iterator_traits : public iterator_traits_helper<Iterator> {};

    // 获得迭代器类型的对象用于标签分派函数重载
    template<typename Iterator>
    typename iterator_traits<Iterator>::iterator_category
        get_iterator_category(const Iterator&)
    {
        typedef typename iterator_traits<Iterator>::iterator_category category;
        return category();
    }

    // 对普通指针的特化版本
    // C++20不再需要const T*的特化版本
    // 特殊迭代器的类型定义由所需容器自行实现
    template<typename T>
    struct iterator_traits<T*>
    {
        using iterator_concept  = contiguous_iterator_tag;
        using iterator_category = random_access_iterator_tag;
        using value_type        = remove_cv_t<T>;
        using difference_type   = ptrdiff_t;
        using pointer           = T*;
        using reference         = T&;
    };

    /*
     * 判断迭代器类型
     */
    template<typename Iterator>
    using iterator_category_t = typename iterator_traits<Iterator>::iterator_category;

    // 若不加上typename = iterator_category_t<Iterator>
    // 在编译器在为vector<int> v3(200, 3);调用构造函数时
    // 匹配constexpr vector(InputIterator first, InputIterator last, const allocator_type& alloc = allocator_type())时
    // 会实例化出iterator_category_t<Iterator>，而不是匹配失败，从而导致编译失败
    template<typename Iterator, typename = iterator_category_t<Iterator>>
    inline constexpr bool is_input_iterator = std::is_convertible_v<iterator_category_t<Iterator>, input_iterator_tag>;

    template<typename Iterator, typename = iterator_category_t<Iterator>>
    inline constexpr bool is_output_iterator = std::is_convertible_v<iterator_category_t<Iterator>, output_iterator_tag>;

    template<typename Iterator, typename = iterator_category_t<Iterator>>
    inline constexpr bool is_forward_iterator = std::is_convertible_v<iterator_category_t<Iterator>, forward_iterator_tag>;

    template<typename Iterator, typename = iterator_category_t<Iterator>>
    inline constexpr bool is_bidirectional_iterator = std::is_convertible_v<iterator_category_t<Iterator>, bidirectional_iterator_tag>;

    template<typename Iterator, typename = iterator_category_t<Iterator>>
    inline constexpr bool is_random_access_iterator = std::is_convertible_v<iterator_category_t<Iterator>, random_access_iterator_tag>;

    template<typename Iterator, typename = iterator_category_t<Iterator>>
    inline constexpr bool is_contiguous_iterator = std::is_convertible_v<iterator_category_t<Iterator>, contiguous_iterator_tag>;

    // 迭代器适配器
    // 可以将普通指针对应的的迭代器转换为类
    // Container用于区分不同容器的相同迭代器类型
    template<typename Iterator, typename Container>
    class iterator_adpater
    {
    private:
        Iterator current;
        using traits_type = iterator_traits<Iterator>;
    
    public:
        using iterator_type     = Iterator;
        using iterator_concept  = typename traits_type::iterator_concept;
        using iterator_category = typename traits_type::iterator_category;
        using value_type        = typename traits_type::value_type;
        using difference_type   = typename traits_type::difference_type;
        using pointer           = typename traits_type::pointer;
        using reference         = typename traits_type::reference;

        constexpr iterator_adpater() noexcept : current(Iterator()) {}
        explicit constexpr iterator_adpater(const Iterator& i) noexcept : current(i) {}

        constexpr reference operator*()
            const noexcept
        {
            return *current;
        }

        constexpr pointer operator->()
            const noexcept
        {
            return current;
        }

        constexpr iterator_adpater& operator++()
            noexcept
        {
            ++current;
            return *this;
        }

        constexpr iterator_adpater operator++(int)
            noexcept
        {
            return iterator_adpater(current++);
        }

        constexpr iterator_adpater& operator--()
            noexcept
        {
            --current;
            return *this;
        }

        constexpr iterator_adpater operator--(int)
            noexcept
        {
            return iterator_adpater(current--);
        }

        constexpr reference operator[](difference_type n)
            const noexcept
        {
            return current[n];
        }

        constexpr iterator_adpater& operator+=(difference_type n)
            noexcept
        {
            current += n;
            return *this;
        }

        constexpr iterator_adpater operator+(difference_type n)
            const noexcept
        {
            return iterator_adpater(current + n);
        }

        constexpr iterator_adpater& operator-=(difference_type n)
            noexcept
        {
            current -= n;
            return *this;
        }

        constexpr iterator_adpater operator-(difference_type n)
            const noexcept
        {
            return iterator_adpater(current - n);
        }

        constexpr const Iterator& base()
            const noexcept
        {
            return current;
        }

        constexpr difference_type operator-(const iterator_adpater& other)
            const noexcept
        {
            return current - other.base();
        }

        constexpr bool operator==(const iterator_adpater& other)
            const noexcept
        {
            return current == other.base();
        }

        constexpr bool operator!=(const iterator_adpater& other)
            const noexcept
        {
            return current != other.base();
        }

        constexpr bool operator<(const iterator_adpater& other)
            const noexcept
        {
            return current < other.base();
        }

        constexpr bool operator<=(const iterator_adpater& other)
            const noexcept
        {
            return current <= other.base();
        }

        constexpr bool operator>(const iterator_adpater& other)
            const noexcept
        {
            return current > other.base();
        }

        constexpr bool operator>=(const iterator_adpater& other)
            const noexcept
        {
            return current >= other.base();
        }
    };

    // 反向迭代器
    template<typename Iterator>
    class reverse_iterator
    {
    private:
        Iterator current;
        using traits_type = iterator_traits<Iterator>;

    public:
        using iterator_type     = Iterator;
        using iterator_concept  = typename traits_type::iterator_concept;
        using iterator_category = typename traits_type::iterator_category;
        using value_type        = typename traits_type::value_type;
        using difference_type   = typename traits_type::difference_type;
        using pointer           = typename traits_type::pointer;
        using reference         = typename traits_type::reference;

        constexpr reverse_iterator() 
            NOEXCEPT_IF(noexcept(Iterator()))
            : current() {}

        explicit constexpr reverse_iterator(iterator_type x) 
            NOEXCEPT_IF(noexcept(Iterator(x)))
            : current(x) {}

        constexpr reverse_iterator(const reverse_iterator& x)
            NOEXCEPT_IF(noexcept(x.current))
            : current(x.current) {}

        reverse_iterator& operator=(const reverse_iterator&) = default;

        [[nodiscard]]
        constexpr iterator_type base()
            const NOEXCEPT_IF(noexcept(Iterator(current)))
        {
            return current;
        }

        // 返回--current处的值
        [[nodiscard]]
        constexpr reference operator*()
            const
        {
            Iterator tmp = current;
            return *--tmp;
        }

        [[nodiscard]]
        constexpr pointer operator->()
            const
        {
            Iterator tmp = current;
            return --tmp;
        }

        constexpr reverse_iterator& operator++()
        {
            --current;
            return *this;
        }

        constexpr reverse_iterator& operator++(int)
        {
            reverse_iterator tmp = *this;
            --current;
            return tmp;
        }

        constexpr reverse_iterator& operator--()
        {
            ++current;
            return *this;
        }

        constexpr reverse_iterator& operator--(int)
        {
            reverse_iterator tmp = *this;
            ++current;
            return tmp;
        }

        [[nodiscard]]
        constexpr reverse_iterator operator+(difference_type n) 
            const
        {
            return reverse_iterator(current - n);
        }

        constexpr reverse_iterator& operator+=(difference_type n)
        {
            current -= n;
            return *this;
        }

        [[nodiscard]]
        constexpr reverse_iterator operator-(difference_type n)
            const
        {
            return reverse_iterator(current + n);
        }

        constexpr reverse_iterator& operator-=(difference_type n)
        {
            current += n;
            return *this;
        }

        [[nodiscard]]
        constexpr reference operator[](difference_type n) 
            const
        {
            return *(*this + n);
        }
    };

    template<typename IteratorL, typename IteratorR>
    [[nodiscard]]
    constexpr bool operator==(const reverse_iterator<IteratorL>& lhs, 
        const reverse_iterator<IteratorR>& rhs)
    { 
        return lhs.base() == rhs.base(); 
    }

    template<typename IteratorL, typename IteratorR>
    [[nodiscard]]
    constexpr bool operator!=(const reverse_iterator<IteratorL>& lhs,
            const reverse_iterator<IteratorR>& rhs)
    { 
        return lhs.base() != rhs.base(); 
    }

    template<typename IteratorL, typename IteratorR>
    [[nodiscard]]
    constexpr bool operator<(const reverse_iterator<IteratorL>& lhs,
            const reverse_iterator<IteratorR>& rhs)
    {
        return lhs.base() > rhs.base();
    }

    template<typename IteratorL, typename IteratorR>
    [[nodiscard]]
    constexpr bool operator<=(const reverse_iterator<IteratorL>& lhs,
            const reverse_iterator<IteratorR>& rhs)
    {
        return lhs.base() >= rhs.base();
    }

    template<typename IteratorL, typename IteratorR>
    [[nodiscard]]
    constexpr bool operator>(const reverse_iterator<IteratorL>& lhs,
            const reverse_iterator<IteratorR>& rhs)
    { 
        return lhs.base() < rhs.base();
    }

    template<typename IteratorL, typename IteratorR>
    [[nodiscard]]
    constexpr bool operator>=(const reverse_iterator<IteratorL>& lhs,
            const reverse_iterator<IteratorR>& rhs)
    { 
        return lhs.base() <= rhs.base();
    }

    /*
     * 计算迭代器间距离 
     */
    // input_iterator_tag
    template<typename InputIterator>
    typename iterator_traits<InputIterator>::difference_type
        distance_dispatch(InputIterator first, InputIterator last, input_iterator_tag)
    {
        typename iterator_traits<InputIterator>::difference_type n = 0;
        while (first != last)
        {
            ++first;
            ++n;
        }
        return n;
    }

    // random_access_iterator_tag
    template<typename RandomIterator>
    typename iterator_traits<RandomIterator>::difference_type
        distance_dispatch(RandomIterator first, RandomIterator last, random_access_iterator_tag)
    {
        return last - first;
    }

    // 统一接口
    template<typename InputIterator>
    typename iterator_traits<InputIterator>::difference_type
        distance(InputIterator first, InputIterator last)
    {
        return distance_dispatch(first, last, get_iterator_category(first));
    }
}

#endif // !ITERATOR_H



