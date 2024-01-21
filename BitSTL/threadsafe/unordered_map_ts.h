/*
 * unordered_map_ts类
 */
#ifndef UNORDERED_MAP_H
#define UNORDERED_MAP_H

#include <memory>
#include <mutex>
#include <shared_mutex>
#include <functional>
#include <map>
#include <list>

namespace bitstl
{
    template<typename K, typename V, typename Hash = std::hash<K>>
    class unordered_map_ts
    {
    private:
        // unordered_map_ts由多个bucket组成,bucket数目为质数最佳。
        class bucket_type;
        std::vector<std::unique_ptr<bucket_type>> buckets_;

        Hash hasher_;

    private:
        // 仅需在bucket一级加锁
        bucket_type& get_bucket(const K& key)
            const
        {
            const std::size_t bucket_index = hasher_(key) % buckets_.size();
            return *buckets_[bucket_index];
        }

    public:
        unordered_map_ts(unsigned buckets_num = 17, const Hash& hasher_ = Hash())
            : buckets_(buckets_num), hasher_(hasher_)
        {
            for (unsigned i = 0; i < buckets_num; ++i)
            {
                buckets_[i].reset(new bucket_type);
            }
        }

        unordered_map_ts(const unordered_map_ts& other) = delete;
        unordered_map_ts& operator=(const unordered_map_ts& other) = delete;

        V get_value(const K& key, const V& default_value = V())
            const
        {
            return get_bucket(key).get_value(key, default_value);
        }

        void add_or_update_value(const K& key, const V& value)
        {
            get_bucket(key).add_or_update_value(key, value);
        }

        bool remove_value(K key)
        {
            return get_bucket(key).remove_value(key);
        }

    private:
        typedef std::pair<K, V> bucket_value;
        typedef std::list<bucket_value> bucket_data;
        typedef typename bucket_data::iterator bucket_iterator;

        class bucket_type
        {
        private:
            bucket_data data_;
            mutable std::shared_mutex smtx_;

        private:
            bucket_iterator find_entry_for(const K& key)
                // const使data_为const，使find_if返回const_iterator
            {
                return std::find_if(data_.begin(), data_.end(),
                    [&](const bucket_value& item) {return item.first == key; }
                );
            }

        public:
            V get_value(const K& key, const V& default_value)
            {
                std::shared_lock<std::shared_mutex> lock(smtx_);
                bucket_iterator found_entry = find_entry_for(key);
                return (found_entry == data_.end()) ? default_value : found_entry->second;
            }

            void add_or_update_value(const K& key, const V& value)
            {
                std::unique_lock<std::shared_mutex> lock(smtx_);
                bucket_iterator found_entry = find_entry_for(key);
                if (found_entry == data_.end())
                {
                    data_.push_back(bucket_value(key, value));
                }
                else
                {
                    found_entry->second = value;
                }
            }

            bool remove_value(const K& key)
            {
                std::unique_lock<std::shared_mutex> lock(smtx_);
                bucket_iterator found_entry = find_entry_for(key);
                if (found_entry != data_.end())
                {
                    data_.erase(found_entry);
                    return true;
                }
                return false;
            }
        };

        // 返回snapshot
        std::map<K, V> get_map()
            const
        {
            std::vector<std::unique_lock<std::shared_mutex>> locks;
            for (unsigned i = 0; i < buckets_.size(); ++i)
            {
                locks.push_back(std::unique_lock<std::shared_mutex>(buckets_[i].mutex));
            }

            std::map<K, V> res;
            for (unsigned i = 0; i < buckets_.size(); ++i)
            {
                for (bucket_iterator it = buckets_[i].data.begin();
                    it != buckets_[i].data.end();
                    ++it)
                {
                    res.insert(*it);
                }
            }
            return res;
        }
    };
}
#endif // !UNORDERED_MAP_H