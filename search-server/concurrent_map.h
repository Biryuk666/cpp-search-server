#pragma once

#include <map>
#include <mutex>
#include <string>
#include <vector>

using namespace std::literals;

template <typename Key, typename Value>
class ConcurrentMap {
private:
    struct Bucket {
        std::map<Key, Value> map;
        std::mutex mutex;
    };
    
    std::vector<Bucket> buckets_;
    
public:
    struct Access {
        std::lock_guard<std::mutex> guard;
        Value& ref_to_value;

        Value& operator +=(const Value& value) {
            return ref_to_value += value;
        }
    };     

    explicit ConcurrentMap(size_t bucket_count) : buckets_(bucket_count) {}

    Access operator[](const Key& key) {
        auto& bucket = buckets_[static_cast<uint64_t>(key) % buckets_.size()];
        return {std::lock_guard(bucket.mutex), bucket.map[key]};
    }

    std::map<Key, Value> BuildOrdinaryMap() {
        std::map<Key, Value> result;
        for (auto& [map, mutex] : buckets_) {
            std::lock_guard guard(mutex);
            result.insert(map.begin(), map.end());
        }
        return result;
    }

    void erase(const Key& key){    
        auto& bucket = buckets_[static_cast<uint64_t>(key) % buckets_.size()];
        std::lock_guard guard(bucket.mutex);
        bucket.map.erase(key);
    }
};