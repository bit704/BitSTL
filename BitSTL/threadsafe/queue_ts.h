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
        std::queue<std::shared_ptr<T>> data;
        mutable std::mutex mtx;
        std::condition_variable cond;

    public:
        queue_ts() = default;

        queue_ts(const queue_ts& other)
        {
            std::lock_guard<std::mutex> lock(other.mtx);
            data = std::move(other.data);
        }

        queue_ts& operator=(const queue_ts&) = delete;

        void push(T new_value)
        {
            // new_value_p构造放在临界区外，缩短持锁时长
            auto new_value_p(std::make_shared<T>(std::move(new_value)));
            std::lock_guard<std::mutex> lock(mtx);
            data.push(new_value_p);
            cond.notify_one();
        }

        void wait_and_pop(T& value)
        {
            std::unique_lock<std::mutex> lock(mtx);
            cond.wait(lock, [this] { return !data.empty(); });
            value = std::move(*data.front());
            data.pop();
        }

        std::shared_ptr<T> wait_and_pop()
        {
            std::unique_lock<std::mutex> lock(mtx);
            cond.wait(lock, [] { return !data.empty(); });
            // data中储存shared_ptr，可直接拷贝赋值。否则需要构造，可能在此抛出异常（如内存不足）。
            auto res = data.front();
            data.pop();
            return res;
        }

        bool try_pop(T& value)
        {
            std::lock_guard<std::mutex> lock(mtx);
            if (data.empty)
                return false;
            value = std::move(*data.front());
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

