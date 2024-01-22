/*
 * queue_ts类
 * 队列
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
        struct node
        {
            std::shared_ptr<T> data;
            std::unique_ptr<node> next;
        };

        mutable std::mutex head_mtx_;
        mutable std::mutex tail_mtx_;

        std::unique_ptr<node> head_;
        node* tail_;
        
        std::condition_variable cond_;

    public:
        queue_ts() : head_(new node), tail_(head_.get()) {}

        queue_ts(const queue_ts& other) = delete;
        queue_ts& operator=(const queue_ts&) = delete;

        void push(T new_value)
        {
            auto new_value_p(std::make_shared<T>(std::move(new_value)));
            
            std::unique_ptr<node> p(new node);
            // 临界区
            {
                std::lock_guard<std::mutex> tail_lock(tail_mtx_);
                tail_->data = new_value_p;
                node* const new_tail = p.get();
                tail_->next = std::move(p);
                tail_ = new_tail;
            }
            cond_.notify_one();
        }

        void wait_and_pop(T& value)
        {
            const std::unique_ptr<node> old_head = wait_pop_head(value);
        }

        std::shared_ptr<T> wait_and_pop()
        {
            const std::unique_ptr<node> old_head = wait_pop_head();
            return old_head->data;
        }

        bool try_pop(T& value)
        {
            const std::unique_ptr<node> old_head = try_pop_head(value);
            return old_head;
        }

        std::shared_ptr<T> try_pop()
        {
            const std::unique_ptr<node> old_head = try_pop_head();
            return old_head ? old_head->data : std::shared_ptr<T>();
        }

        bool empty()
        {
            std::lock_guard<std::mutex> head_lock(head_mtx_);
            return head_.get() == get_tail();
        }

    private:
        node* get_tail()
        {
            std::lock_guard<std::mutex> tail_lock(tail_mtx_);
            return tail_;
        }

        std::unique_ptr<node> pop_head()
        {
            std::unique_ptr<node> old_head = std::move(head_);
            head_ = std::move(old_head->next);
            return old_head;
        }

        std::unique_lock<std::mutex> wait_for_data()
        {
            std::unique_lock<std::mutex> head_lock(head_mtx_);
            cond_.wait(head_lock, [&] {return head_.get() != get_tail(); });
            return std::move(head_lock);
        }

        std::unique_ptr<node> wait_pop_head()
        {
            std::unique_lock<std::mutex> head_lock(wait_for_data());
            return pop_head();
        }

        std::unique_ptr<node> wait_pop_head(T& value)
        {
            std::unique_lock<std::mutex> head_lock(wait_for_data());
            value = std::move(*head_->data);
            return pop_head();
        }

        std::unique_ptr<node> try_pop_head()
        {
            std::unique_lock<std::mutex> head_lock(head_mtx_);
            if (head_.get() == get_tail())
            {
                return std::unique_ptr<node>();
            }
            return pop_head();
        }

        std::unique_ptr<node> try_pop_head(T& value)
        {
            std::unique_lock<std::mutex> head_lock(head_mtx_);
            if (head_.get() == get_tail())
            {
                return std::unique_ptr<node>();
            }
            value = std::move(*head_->data);
            return pop_head();
        }
    };
}
#endif // !QUEUE_TS_H

