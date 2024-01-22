/*
 * stack_ts类
 * 无锁栈
 */
#ifndef STACK_TS_H
#define STACK_TS_H

#include <atomic>
#include <memory>

namespace bitstl
{
    template<typename T>
    class stack_ts
    {
    private:
        struct node
        {
            std::shared_ptr<T> data;
            node* next;
            node(const T& _data) : data(std::make_shared<T>(_data)) {}
        };
        std::atomic<node*> head_;

    public:
        void push(const T& new_value)
        {
            node* const new_node = new node(new_value);
            new_node->next = head_.load();
            // 当head未被其它指针改动过时更新head_
            while (!head_.compare_exchange_weak(new_node->next, new_node));
        }

        std::shared_ptr<T> pop()
        {
            ++threads_popping_;
            node* old_head = head_.load();
            // 当head未被其它指针改动过时更新head_
            while (old_head && !head_.compare_exchange_weak(old_head, old_head->next));
            std::shared_ptr<T> res;
            if (old_head)
                res.swap(old_head->data);
            try_delete(old_head);
            return res;
        }

        bool empty()
            const
        {
            return head_.load() == nullptr;
        }

    private:
        std::atomic<unsigned int> threads_popping_; // 当前使用pop函数的线程数量
        std::atomic<node*> delete_candidate_; 

        void try_delete(node * old_head)
        {
            // 高并发场景下threads_popping_一直不为1，导致delete_candidate_无限增加，得不到释放
            // 实际上没有线程访问的node即可释放，使用hazard pointer、引用计数能够实现
            if (threads_popping_ == 1)
            {
                node* candidates = delete_candidate_.exchange(nullptr);
                if (!(--threads_popping_)) // threads_in_pop为0，无线程调用pop
                {
                    delete_nodes(candidates);
                }
                else if (candidates)
                {
                    add_candidates(candidates);
                }
                delete old_head; // 先尝试删除delete_candidate_，再删除old_head
            }
            else
            {
                add_candidate(old_head);
                --threads_popping_;
            }
        }

        static void delete_nodes(node* ns)
        {
            while (ns)
            {
                node* next = ns->next;
                delete ns;
                ns = next;
            }
        }

        void add_candidate(node* n)
        {
            add_candidates(n, n);
        }

        void add_candidates(node* first, node* last)
        {
            last->next = delete_candidate_;
            while (!delete_candidate_.compare_exchange_weak(last->next, first));
        }

        void add_candidates(node* ns)
        {
            node* last = ns;
            // 将last移动到链表末端
            while (node* const next = last->next) 
            {
                last = next;
            }
            add_candidates(ns, last);
        }
    };
}
#endif // !STACK_TS_H