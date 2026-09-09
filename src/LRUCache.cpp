#include "LRUCache.h"

namespace searchx {

bool LRUCache::get(const std::string& key, std::vector<ScoredDoc>& outValue) {
    auto it = lookup_.find(key);
    if (it == lookup_.end()) {
        ++misses_;
        return false;
    }
    ++hits_;
    items_.splice(items_.begin(), items_, it->second); // bump to most-recently-used
    outValue = it->second->second;
    return true;
}

void LRUCache::put(const std::string& key, const std::vector<ScoredDoc>& value) {
    auto it = lookup_.find(key);
    if (it != lookup_.end()) {
        it->second->second = value;
        items_.splice(items_.begin(), items_, it->second);
        return;
    }

    if (items_.size() >= capacity_) {
        auto& [oldKey, oldValue] = items_.back();
        lookup_.erase(oldKey);
        items_.pop_back();
    }

    items_.emplace_front(key, value);
    lookup_[key] = items_.begin();
}

} // namespace searchx
