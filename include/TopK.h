#pragma once
#include <vector>
#include "Ranker.h"

namespace searchx {

// Picks the k highest-scoring docs without fully sorting everything.
class TopK {
public:
    // O(n log k): one pass over n candidates, each heap op costs log k.
    // Plain sort would be O(n log n) -- worse whenever k << n, which is the
    // normal case (thousands of matches, only 10 shown).
    static std::vector<ScoredDoc> select(const std::vector<ScoredDoc>& scored, size_t k);
};

} // namespace searchx
