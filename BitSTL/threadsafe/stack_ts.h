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
        std::stack<T> data;
        mutable std::mutex mtx;

    public:
        stack_ts() = default;

        stack_ts(const stack_ts& other)
        {
            std::lock_guard<std::mutex> lock(other.mtx);
            data = other.data;
        }

        stack_ts& operator=(const stack_ts&) = delete;

        void push(T new_value)
        {
            std::lock_guard<std::mutex> lock(mtx);
            data.push(std::move(new_value));
        }

        // 返回智能指针
        std::shared_ptr<T> pop()
        {
            std::lock_guard<std::mutex> lock(mtx);
            if (data.empty())
                return std::shared_ptr<T>();
            auto res = std::make_shared<T>(data.top());
            data.pop();
            return res;
        }

        // 返回引用
        bool pop(T& value)
        {
            std::lock_guard<std::mutex> lock(mtx);
            if (data.empty())
                return false;
            value = data.top();
            return true;
        }

        bool empty()
            const
        {
            std::lock_guard<std::mutex> lock(mtx);
            return data.empty();
        }
    };

}
#endif // !STACK_TS_H