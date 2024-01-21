/*
 * stack_ts类
 */
#ifndef STACK_TS_H
#define STACK_TS_H

#include <exception>
#include <memory>
#include <mutex>
#include <stack>

namespace bitstl
{
    template<typename T>
    class stack_ts
    {
    private:
        std::stack<std::shared_ptr<T>> data_;
        mutable std::mutex mtx_;

    public:
        stack_ts() = default;

        stack_ts(const stack_ts& other)
        {
            std::lock_guard<std::mutex> lock(other.mtx_);
            data_ = std::move(other.data_);
        }

        stack_ts& operator=(const stack_ts&) = delete;

        void push(T new_value)
        {
            auto new_value_p(std::make_shared<T>(std::move(new_value)));
            std::lock_guard<std::mutex> lock(mtx_);
            data_.push(new_value_p);
        }

        // 返回智能指针
        std::shared_ptr<T> pop()
        {
            std::lock_guard<std::mutex> lock(mtx_);
            if (data_.empty())
                return std::shared_ptr<T>();
            auto res = data_.top();
            data_.pop();
            return res;
        }

        // 返回引用
        bool pop(T& value)
        {
            std::lock_guard<std::mutex> lock(mtx_);
            if (data_.empty())
                return false;
            value = std::move(*data_.top());
            return true;
        }

        bool empty()
            const
        {
            std::lock_guard<std::mutex> lock(mtx_);
            return data_.empty();
        }
    };

}
#endif // !STACK_TS_H