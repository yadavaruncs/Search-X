#pragma once
#include <list>
#include <string>
#include <unordered_map>
#include <vector>
#include "Ranker.h"

namespace searchx {

// Fixed-capacity Least-Recently-Used cache: query string -> its ranked
// results. A doubly linked list tracks recency (front = most recently
// used, back = next to evict); a hash map gives O(1) lookup straight to
// that item's list node. Together get() and put() are both O(1).
class LRUCache {
public:
    explicit LRUCache(size_t capacity) : capacity_(capacity) {}

    bool get(const std::string& key, std::vector<ScoredDoc>& outValue);
    void put(const std::string& key, const std::vector<ScoredDoc>& value);

    size_t hits() const { return hits_; }
    size_t misses() const { return misses_; }

private:
    using Entry = std::pair<std::string, std::vector<ScoredDoc>>;

    size_t capacity_;
    std::list<Entry> items_; // front = most recently used, back = least
    std::unordered_map<std::string, std::list<Entry>::iterator> lookup_;
    size_t hits_ = 0;
    size_t misses_ = 0;
};

} // namespace searchx
