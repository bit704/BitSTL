/*
 * 动态连续数组 
 */
#ifndef VECTOR_H
#define VECTOR_H

#include "allocator.h"
#include "allocator_traits.h"
#include "iterator.h"

namespace bitstl
{
    // vector protected继承自 vector_base
    // 被继承时，对没有数据成员的对象alloc实现EBO（空基类优化）
    // vector_base仅进行内存的分配和释放，vector进行对象的构造和析构，解耦
    template<typename T, typename Alloc>
    struct vector_base
    {
        using allocator_type = Alloc;
        using allocator_traits_type = allocator_traits<allocator_type>;
        using pointer  = typename allocator_traits_type::pointer;

        pointer start;          // 头元素
        pointer finish;         // 尾元素
        pointer end_of_storage; // 储存空间尾部
        allocator_type alloc;   // 内存分配器萃取接口
        
        constexpr vector_base() NOEXCEPT_IF(noexcept(allocator_type())) 
            : start(pointer()), finish(pointer()), end_of_storage(pointer()), alloc(allocator_type()) {}

        constexpr vector_base(const allocator_type& _alloc)
            : start(pointer()), finish(pointer()), end_of_storage(pointer()), alloc(_alloc) {}

        constexpr vector_base(const pointer& _start, const pointer& _finish, const pointer& _end_of_storage)
            : start(_start), finish(_finish), end_of_storage(_end_of_storage), alloc(allocator_type()) {}

        constexpr vector_base(const pointer& _start, const pointer& _finish, const pointer& _end_of_storage, const allocator_type& _alloc)
            : start(_start), finish(_finish), end_of_storage(_end_of_storage), alloc(_alloc) {}

        constexpr pointer allocate(size_t n)
        {
            if (n)
                return allocator_traits_type::allocate(alloc, n);
            else
                return pointer();
        }

        constexpr void deallocate(pointer p, size_t n)
        {
            if (p)
                allocator_traits_type::deallocate(alloc, p, n);
        }        
    };

    template<typename T, typename Alloc = allocator<T>>
    class vector : protected vector_base<T, Alloc>
    {
        static_assert(is_same_v<typename remove_cv<T>::type, T>,
            "vector must have a non-const, non-volatile value_type");

        static_assert(is_same_v<typename Alloc::value_type, T>,
            "vector must have the same value_type as its allocator");

    private:
        using base = vector_base<T, Alloc>;
        using Alloc_traits = allocator_traits<Alloc>;

    public:
        /*
         * 成员类型 
         */
        using value_type      = T;
        using allocator_type  = Alloc;
        using size_type       = size_t;
        using difference_type = ptrdiff_t;
        using reference       = value_type&;
        using const_reference = const value_type&;
        using pointer         = typename Alloc_traits::pointer;
        using const_pointer   = typename Alloc_traits::const_pointer;
        // 迭代器
        using iterator        = iterator_adpater<pointer, vector>;
        using const_iterator  = iterator_adpater<const_pointer, vector>;        
        using reverse_iterator       = reverse_iterator<iterator>;
        // 不加bitstl::会使用上面定义的reverse_iterator而不是reverse_iterator模板，导致编译错误
        using const_reverse_iterator = bitstl::reverse_iterator<const_iterator>;                
        
        /*
         * 构造函数 
         */
        constexpr vector() NOEXCEPT_IF(noexcept(allocator_type())) = default;

        constexpr explicit vector(const allocator_type& alloc) 
            noexcept 
            : base(alloc) {}

        constexpr vector(size_type count, const value_type& value, const allocator_type& alloc = allocator_type())
            : base(alloc)
        {
            fill_count(count, value);
        }

        explicit constexpr vector(size_type count, const allocator_type& alloc = allocator_type())
            : base(alloc)
        {
            fill_count(count, value_type());
        }

        // 写法1：使用非类型模板参数
        //template<
        //    typename InputIterator,
        //    typename enable_if_t<is_input_iterator<InputIterator>, int> = 0
        //>
        // 写法2：使用类型模板参数
        template<
            typename InputIterator,
            typename = enable_if_t<is_input_iterator<InputIterator>>
        >
        constexpr vector(InputIterator first, InputIterator last, const allocator_type& alloc = allocator_type())
            : base(alloc)
        {
            assert(first < last);
            fill_range(first, last);
        }

        constexpr vector(const vector& other)
        {
            fill_range(other.start, other.finish);
        }

        constexpr vector(const vector& other, const allocator_type& alloc)
            : base(alloc)
        {
            fill_range(other.start, other.finish);
        }

        constexpr vector(vector&& other)
            noexcept
            : base(other.start, other.finish, other.end_of_storage)
        {
            other.start = other.finish = other.end_of_storage = nullptr;
        }

        constexpr vector(vector&& other, const allocator_type& alloc)
            : base(other.start, other.finish, other.end_of_storage, alloc)
        {
            other.start = other.finish = other.end_of_storage = nullptr;
        }

        constexpr vector(std::initializer_list<T> init, const allocator_type& alloc = allocator_type())
            : base(alloc)
        {
            fill_range(init.begin(), init.end());
        }

        /*
         * 析构函数 
         */
        constexpr ~vector()
        {
            for (pointer cur = this->start; cur < this->finish; ++cur)
                Alloc_traits::destroy(this->alloc, cur);
            this->deallocate(this->start, this->end_of_storage - this->start);
            this->start = this->finish = this->end_of_storage = nullptr;
        }

        /*
         * 重载operator=
         */
        constexpr vector& operator=(const vector& other)
        {
            if (this != &other)
            {
                size_type new_size = other.size();
                // new_size大于当前capacity
                if (new_size > capacity())
                {
                    vector tmp(other.begin(), other.end());
                    swap(tmp);
                }
                // new_size位于capacity和size之间
                else if (new_size > size())
                {
                    copy(other.begin(), other.begin() + size(), begin());
                    uninitialized_copy(other.begin() + size(), other.end(), this->finish);
                    this->end_of_storage = this->finish = this->start + new_size;
                }
                // new_size小于当前size
                else
                {
                    auto copy_end = copy(other.begin(), other.end(), begin());
                    for (auto cur = &*copy_end; cur < this->finish; ++cur)
                        Alloc_traits::destroy(this->alloc, cur);
                    this->finish = this->start + new_size;
                }
            }
            return *this;
        }

        constexpr vector& operator=(vector&& other)
            noexcept
        {
            if (this != &other)
            {
                size_type new_size = other.size();

                for (pointer cur = this->start; cur < this->finish; ++cur)
                    Alloc_traits::destroy(this->alloc, cur);
                this->deallocate(this->start, this->end_of_storage - this->start);

                this->start = other.start;
                this->finish = other.finish;
                this->end_of_storage = other.end_of_storage;
                other.start = nullptr;
                other.finish = nullptr;
                other.end_of_storage = nullptr;
                return *this;
            }
        }

        constexpr vector& operator=(std::initializer_list<T> ilist)
        {
            vector tmp(ilist.begin(), ilist.end());
            swap(tmp);
            return *this;
        }

        /*
         * assign函数
         */
        constexpr void assign(size_type count, const T& value)
        {
            assign_count(count, value);
        }

        template<
            typename InputIterator,
            typename = enable_if_t<is_input_iterator<InputIterator>>
        >
        constexpr void assign(InputIterator first, InputIterator last)
        {
            vector tmp = vector(first, last);
            swap(tmp);
        }

        constexpr void assign(std::initializer_list<T> ilist)
        {
            vector tmp = vector(ilist);
            swap(tmp);
        }
        
        /*
         * 获取allocator 
         */
        constexpr allocator_type get_allocator() 
            const noexcept
        {
            return this->alloc;
        }

        /*
         * 元素访问
         * at执行边界检查，[]不执行
         */
        reference at(size_type pos)
        {
            assert(pos < size());
            return (*this)[pos];
        }

        const_reference at(size_type pos) 
            const
        {
            assert(pos < size());
            return (*this)[pos];
        }

        reference operator[](size_type n)
        {
            return *(this->start + n);
        }

        const_reference operator[](size_type n) 
            const
        {
            return *(this->start + n);
        }

        reference front()
        {
            return *begin();
        }

        const_reference front()
            const
        {
            return *begin();
        }

        reference back()
        {
            return *(end() - 1);
        }

        const_reference back()
            const
        {
            return *(end() - 1);
        }

        T* data()
            noexcept
        {
            return this->start;
        }

        const T* data()
            const
        {
            return this->start;
        }

        /*
         * 迭代器 
         */
        iterator begin()
            noexcept
        {
            return iterator(this->start);
        }

        const_iterator begin()
            const noexcept
        {
            return const_iterator(this->start);
        }

        const_iterator cbegin()
            const noexcept
        {
            return const_iterator(this->start);
        }

        iterator end()
            noexcept
        {
            return iterator(this->finish);
        }

        const_iterator end()
            const noexcept
        {
            return const_iterator(this->finish);
        }

        const_iterator cend()
            const noexcept
        {
            return const_iterator(this->finish);
        }

        reverse_iterator rbegin()
            noexcept
        {
            return reverse_iterator(end());
        }

        const_reverse_iterator rbegin()
            const noexcept
        {
            return const_reverse_iterator(end());
        }

        const_reverse_iterator crbegin()
            const noexcept
        {
            return const_reverse_iterator(end());
        }

        reverse_iterator rend()
            noexcept
        {
            return reverse_iterator(begin());
        }

        const_reverse_iterator rend()
            const noexcept
        {
            return const_reverse_iterator(begin());
        }

        const_reverse_iterator crend()
            const noexcept
        {
            return const_reverse_iterator(begin());
        }

        /*
         * 容量相关
         */
        [[nodiscard]] 
        constexpr bool empty() 
            const noexcept
        {
            return this->start == this->finish;
        }

        constexpr size_type size() 
            const noexcept
        {
            return static_cast<size_type>(this->finish - this->start);
        }

        constexpr size_type max_size() 
            const noexcept
        {
            return static_cast<size_type>(-1) / sizeof(value_type);
        }

        constexpr void reserve(size_type new_cap)
        {
            if (capacity() < new_cap)
            {
                auto old_size = size();
                auto new_start = this->allocate(new_cap);
                uninitialized_move(this->start, this->finish, new_start);
                this->deallocate(this->start, this->end_of_storage - this->start);
                this->start = new_start;
                this->finish = new_start + old_size;
                this->end_of_storage = new_start + new_cap;
            }
        }

        constexpr size_type capacity() 
            const noexcept
        {
            return static_cast<size_type>(this->end_of_storage - this->start);
        }

        constexpr void shrink_to_fit()
        {
            if (this->finish < this->end_of_storage)
            {
                size_t new_size = size();
                auto new_start = this->allocate(new_size);
                uninitialized_move(this->start, this->finish, new_start);
                this->deallocate(this->start, this->end_of_storage - this->start);
                this->start = new_start;
                this->finish = this->end_of_storage = new_start + new_size;
            }
        }

        /*
         * 修改元素
         */
        constexpr void clear()
            noexcept
        {
            erase(begin(), end());
        }

        constexpr iterator insert(iterator pos, const T& value)
        {
            difference_type offset = pos - begin();
            if (this->finish == this->end_of_storage)
                grow();
            iterator new_pos = begin() + offset;
            this->finish = move(new_pos, end(), new_pos + 1).base();
            Alloc_traits::construct(this->alloc, new_pos.base(), value);
            return new_pos;
        }

        constexpr iterator insert(iterator pos, T&& value)
        {
            difference_type offset = pos - begin();
            if (this->finish == this->end_of_storage)
                grow();
            iterator new_pos = begin() + offset;
            this->finish = move(new_pos, end(), new_pos + 1).base();
            Alloc_traits::construct(this->alloc, new_pos.base(), move(value));
            return new_pos;
        }

        template<typename... Args>
        constexpr iterator emplace(iterator pos, Args&&... args)
        {
            difference_type offset = pos - begin();
            if (this->finish == this->end_of_storage)
                grow();
            iterator new_pos = begin() + offset;
            this->finish = move(new_pos, end(), new_pos + 1).base();
            Alloc_traits::construct(this->alloc, new_pos.base(), forward<Args>(args)...);
            return new_pos;
        }

        constexpr iterator insert(iterator pos, size_type count, const T& value)
        {
            difference_type offset = pos - begin();
            if (this->finish + count > this->end_of_storage)
                grow(count);
            iterator new_pos = begin() + offset;
            this->finish = move(new_pos, end(), new_pos + count).base();
            uninitialized_fill_n(new_pos, count, value);
            return new_pos;
        }

        template<
            typename InputIterator,
            typename = enable_if_t<is_input_iterator<InputIterator>>
        >
        constexpr iterator insert(iterator pos, InputIterator first, InputIterator last)
        {
            difference_type offset = pos - begin();
            difference_type count = distance(first, last);
            if (this->finish + count > this->end_of_storage)
                grow(count);
            iterator new_pos = begin() + offset;
            this->finish = move(new_pos, end(), new_pos + count).base();
            uninitialized_copy(first, last, new_pos);
            return new_pos;
        }

        constexpr iterator insert(iterator pos, std::initializer_list<T> ilist)
        {
            difference_type offset = pos - begin();
            difference_type count = ilist.size();
            if (this->finish + count > this->end_of_storage)
                grow(count);
            iterator new_pos = begin() + offset;
            this->finish = move(new_pos, end(), new_pos + count).base();
            uninitialized_copy(ilist.begin(), ilist.end(), new_pos);
            return new_pos;
        }

        constexpr iterator erase(iterator pos)
        {
            this->finish = move(pos + 1, end(), pos).base();
            Alloc_traits::destroy(this->alloc, this->finish);
            return pos;
        }

        constexpr iterator erase(iterator first, iterator last)
        {
            if (first != last)
            {
                auto new_finish = move(last, end(), first).base();
                for(auto tmp = new_finish; tmp != this->finish; ++tmp)
                    Alloc_traits::destroy(this->alloc, tmp);
                this->finish = new_finish;
            }
            return first;
        }

        constexpr void push_back(const T& value)
        {
            if (this->finish == this->end_of_storage)
                grow();            
            Alloc_traits::construct(this->alloc, this->finish, value);
            ++this->finish;
        }

        constexpr void push_back(T&& value)
        {
            emplace_back(move(value));
        }

        template<typename... Args>
        constexpr reference emplace_back(Args&&... args)
        {
            if (this->finish == this->end_of_storage)
                grow();
            Alloc_traits::construct(this->alloc, this->finish, forward<Args>(args)...);
            ++this->finish;
            return *(this->finish - 1);
        }

        constexpr void pop_back()
        {
            --this->finish;
            Alloc_traits::destroy(this->alloc, this->finish);
        }

        void resize(size_type count)
        {
            resize(count, value_type());
        }

        void resize(size_type count, const value_type& value)
        {
            assign(count, value);
        }

        constexpr void swap(vector& other) 
            noexcept(allocator_traits<allocator_type>::propagate_on_container_swap::value
                || allocator_traits<allocator_type>::is_always_equal::value)
        {
            if (this != &other)
            {
                bitstl::swap(this->start, other.start);
                bitstl::swap(this->finish, other.finish);
                bitstl::swap(this->end_of_storage, other.end_of_storage);
            }
        }

    private:
        // 调用alloactor从start开始填充
        void try_allocate(size_type count)
        {
            try
            {
                this->start  = this->allocate(count);
                this->finish = this->start + count;
                this->end_of_storage = this->start + count;
            }
            catch (...)
            {
                this->start = nullptr;
                this->finish = nullptr;
                this->end_of_storage = nullptr;
                throw;
            }
        }

        void fill_count(size_type count, const value_type& value)
        {
            try_allocate(count);
            uninitialized_fill_n(this->start, count, value);
        }

        template<typename InputIterator>
        void fill_range(InputIterator first, InputIterator last)
        {
            const size_type len = distance(first, last);
            try_allocate(len);
            uninitialized_copy(first, last, this->start);
        }

        void assign_count(size_type n, const value_type& value)
        {
            if (n > capacity())
            {
                vector tmp(n, value);
                swap(tmp);
            }
            else if (n > size())
            {
                fill(begin(), end(), value);
                const size_type add = n - size();
                this->finish = uninitialized_fill_n(this->finish, add, value);
            }
            else
            {
                auto new_finish = fill_n(this->start, n, value);
                erase(iterator(new_finish), end());
                this->finish = new_finish;
            }
        }

        void grow(size_type count = 1)
        {
            const size_type len = this->finish - this->start;
            reserve(std::max(len * 2, len + count));
        }
    };

    template<typename T>
    constexpr inline void swap(vector<T>& lhs, vector<T>& rhs)
    {
        lhs.swap(rhs);
    }
}

#endif // !VECTOR_H

