/*
 * list_ts类
 */
#ifndef LIST_TS_H
#define LIST_TS_H

#include <memory>
#include <mutex>

namespace bitstl
{
    template<typename T>
    class list_ts
    {
    private:
        struct node
        {
            std::mutex mtx;
            std::shared_ptr<T> data;
            std::unique_ptr<node> next;
            node() : next() {}
            node(const T& value) : data(std::make_shared<T>(value)) {}
        } head_;

    public:
        list_ts() {}

        ~list_ts()
        {
            remove_if([](const node&) { return true; });
        }

        list_ts(const list_ts& other) = delete;
        list_ts& operator=(const list_ts& other) = delete;

        void push_front(const T& value)
        {
            std::unique_ptr<node> new_node(new node(value));
            std::lock_guard<std::mutex> lock(head_.mtx);
            new_node->next = std::move(head_.next);
            head_.next = std::move(new_node);
        }

        bool empty()
        {
            std::lock_guard<std::mutex> lock(head_.mtx);
            return head_.next == nullptr;
        }

        template<typename Function>
        void for_each(Function f)
        {
            node* current = &head_;
            std::unique_lock<std::mutex> lock(head_.mtx);
            // 底层const，防止修改数据
            while (node* const next = current->next.get())
            {
                std::unique_lock<std::mutex> next_lock(next->mtx);
                lock.unlock();
                f(*next->data);
                current = next;
                lock = std::move(next_lock);
            }
        }

        template<typename Predicate>
        std::shared_ptr<T> find_first_if(Predicate p)
        {
            node* current = &head_;
            std::unique_lock<std::mutex> lock(head_.mtx);
            while (node* const next = current->next.get())
            {
                std::unique_lock<std::mutex> next_lock(next->mtx);
                lock.unlock();
                if (p(*next->data))
                {
                    return next->data;
                }
                current = next;
                lock = std::move(next_lock);
            }
            return std::shared_ptr<T>();
        }

        template<typename Predicate>
        void remove_if(Predicate p)
        {
            node * current = &head_;
            std::unique_lock<std::mutex> lock(head_.mtx);
            while (node* const next = current->next.get())
            {
                std::unique_lock<std::mutex> next_lock(next->mtx);
                if (p(*next->data))
                {
                    std::unique_ptr<node> old_next = std::move(current->next);
                    current->next = std::move(next->next);
                    next_lock.unlock();
                }
                else
                {
                    lock.unlock(); 
                    current = next;
                    lock = std::move(next_lock);
                }
            }
        }
    };
}
#endif // !LIST_TS_H