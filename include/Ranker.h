#pragma once
#include <string>
#include <vector>
#include "InvertedIndex.h"

namespace searchx {

struct ScoredDoc {
    int docId;
    double score;
};

// Scores documents against a query using TF-IDF:
//   TF  (term frequency)      -- how often the word appears in this doc
//   IDF (inverse doc frequency) -- rarer words across the corpus count for more
// A doc mentioning a rare query word a lot scores higher than one mentioning
// a common word once.
class Ranker {
public:
    explicit Ranker(const InvertedIndex& index) : index_(index) {}

    std::vector<ScoredDoc> score(const std::vector<std::string>& queryTerms) const;

private:
    const InvertedIndex& index_;
    double idf(const std::string& term) const;
};

} // namespace searchx
