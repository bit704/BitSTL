/*
 * threadsafe_stack类
 */
#ifndef THREADSAFE_STACK_H
#define THREADSAFE_STACK_H

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
class threadsafe_stack
{
private:
    std::stack<T> data;
    mutable std::mutex m;

public:
    threadsafe_stack() = default;

    threadsafe_stack(const threadsafe_stack& other)
    {
        std::lock_guard<std::mutex> lock(other.m);
        data = other.data;
    }

    threadsafe_stack& operator=(const threadsafe_stack&) = delete;

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


#endif // !THREADSAFE_STACK_H