/*
 * queue_ts类
 */
#ifndef QUEUE_TS_H
#define QUEUE_TS_H

#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>

namespace bitstl
{
    template<typename T>
    class queue_ts
    {
    private:
        std::queue<T> data;
        mutable std::mutex mtx;
        std::condition_variable cond;

    public:
        queue_ts() = default;

        queue_ts(const queue_ts& other)
        {
            std::lock_guard<std::mutex> lock(other.mtx);
            data = other.data;
        }

        queue_ts& operator=(const queue_ts&) = delete;

        void push(T new_value)
        {
            std::lock_guard<std::mutex> lock(mtx);
            data.push(new_value);
            cond.notify_one();
        }

        void wait_and_pop(T& value)
        {
            std::unique_lock<std::mutex> lock(mtx);
            cond.wait(lock, [this] { return !data.empty(); });
            value = data.front();
            data.pop();
        }

        std::shared_ptr<T> wait_and_pop()
        {
            std::unique_lock<std::mutex> lock(mtx);
            cond.wait(lock, [] { return !data.empty(); });
            auto res = std::make_shared<T>(data.front());
            data.pop();
            return res;
        }

        bool try_pop(T& value)
        {
            std::lock_guard<std::mutex> lock(mtx);
            if (data.empty)
                return false;
            value = data.front();
            data.pop();
            return true;
        }

        std::shared_ptr<T> try_pop()
        {
            std::lock_guard<std::mutex> lock(mtx);
            if (data.empty)
                return std::shared_ptr<T>();
            auto res = std::make_shared<T>(data.front());
            data.pop();
            return res;
        }

        bool empty()
            const
        {
            std::lock_guard<std::mutex> lock(mtx);
            return data.empty();
        }
    };

}
#endif // !QUEUE_TS_H

