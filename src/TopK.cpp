#include "TopK.h"
#include <algorithm>
#include <queue>

namespace searchx {

std::vector<ScoredDoc> TopK::select(const std::vector<ScoredDoc>& scored, size_t k) {
    // Min-heap capped at size k: the smallest of the "current best k" sits
    // on top, ready to be evicted the moment something bigger shows up.
    auto byScoreAscending = [](const ScoredDoc& a, const ScoredDoc& b) {
        return a.score > b.score; // priority_queue is a max-heap by default; flip it to get a min-heap
    };
    std::priority_queue<ScoredDoc, std::vector<ScoredDoc>, decltype(byScoreAscending)> minHeap(byScoreAscending);

    for (const ScoredDoc& doc : scored) {
        if (minHeap.size() < k) {
            minHeap.push(doc);
        } else if (doc.score > minHeap.top().score) {
            minHeap.pop();
            minHeap.push(doc);
        }
    }

    std::vector<ScoredDoc> result;
    result.reserve(minHeap.size());
    while (!minHeap.empty()) {
        result.push_back(minHeap.top());
        minHeap.pop();
    }

    // Heap only guarantees the *set* of top-k, not their order -- one final
    // sort (O(k log k), cheap since k is small) puts them best-first.
    std::sort(result.begin(), result.end(), [](const ScoredDoc& a, const ScoredDoc& b) {
        return a.score > b.score;
    });
    return result;
}

} // namespace searchx
