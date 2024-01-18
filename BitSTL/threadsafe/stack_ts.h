/*
 * stack_ts类
 */
#ifndef STACK_TS_H
#define STACK_TS_H

#include <exception>
#include <memory>
#include <mutex>
#include <stack>

// 空stack上调用pop()时抛出
struct empty_stack : std::exception 
{
    const char* what()
        const noexcept
    {
        return "empty stack";
    }
};

template<typename T>
class stack_ts
{
private:
    std::stack<T> data;
    mutable std::mutex m;

public:
    stack_ts() = default;

    stack_ts(const stack_ts& other)
    {
        std::lock_guard<std::mutex> lock(other.m);
        data = other.data;
    }

    stack_ts& operator=(const stack_ts&) = delete;

    void push(T new_value)
    {
        std::lock_guard<std::mutex> lock(m);
        data.push(std::move(new_value));
    }

    // 返回智能指针
    std::shared_ptr<T> pop()
    {
        std::lock_guard<std::mutex> lock(m);
        if (data.empty()) 
            throw empty_stack();
        std::shared_ptr<T> const res(std::make_shared<T>(data.top()));
        data.pop();
        return res;
    }

    // 返回引用
    void pop(T& value)
    {
        std::lock_guard<std::mutex> lock(m);
        if(data.empty()) 
            throw empty_stack();
        value = data.top();
        return;
    }

    bool empty()
        const
    {
        std::lock_guard<std::mutex> lock(m);
        return data.empty();
    }
};


#endif // !STACK_TS_H