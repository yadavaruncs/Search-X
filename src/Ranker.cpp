#include "Ranker.h"
#include <cmath>
#include <unordered_map>

namespace searchx {

double Ranker::idf(const std::string& term) const {
    int N = index_.documentCount();
    int df = index_.documentFrequency(term);
    if (df == 0) return 0.0;
    // +1 smoothing keeps this defined (and non-negative) even when df == N.
    return std::log(static_cast<double>(N + 1) / (df + 1)) + 1.0;
}

std::vector<ScoredDoc> Ranker::score(const std::vector<std::string>& queryTerms) const {
    std::unordered_map<int, double> totals;

    for (const std::string& term : queryTerms) {
        const auto* postings = index_.getPostings(term);
        if (!postings) continue;

        double weight = idf(term);
        for (const auto& [docId, termFreq] : *postings) {
            totals[docId] += termFreq * weight;
        }
    }

    std::vector<ScoredDoc> results;
    results.reserve(totals.size());
    for (const auto& [docId, score] : totals) {
        results.push_back({docId, score});
    }
    return results;
}

} // namespace searchx
