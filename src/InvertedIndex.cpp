#include "InvertedIndex.h"
#include "Tokenizer.h"
#include <algorithm>
#include <future>

namespace searchx {

// Each thread tokenizes its slice of documents and builds its own local
// index -- no shared state, so no locking needed during this phase.
std::unordered_map<std::string, InvertedIndex::PostingList> InvertedIndex::buildPartial(
    const std::vector<Document>& docs, size_t begin, size_t end) {
    std::unordered_map<std::string, PostingList> partial;
    for (size_t i = begin; i < end; ++i) {
        const Document& doc = docs[i];
        for (const std::string& term : Tokenizer::tokenize(doc.text)) {
            partial[term][doc.id]++;
        }
    }
    return partial;
}

// Folds one thread's partial index into the final index. Runs on the main
// thread after all workers finish, so this part stays single-threaded and
// needs no synchronization.
void InvertedIndex::merge(std::unordered_map<std::string, PostingList>&& partial) {
    for (auto& [term, postings] : partial) {
        auto& target = index_[term];
        for (auto& [docId, freq] : postings) {
            target[docId] += freq;
        }
    }
}

void InvertedIndex::build(const std::vector<Document>& docs, unsigned numThreads) {
    documents_ = docs;
    index_.clear();

    numThreads = std::max(1u, numThreads);
    size_t n = docs.size();
    size_t chunkSize = (n + numThreads - 1) / numThreads;

    std::vector<std::future<std::unordered_map<std::string, PostingList>>> futures;
    for (unsigned t = 0; t < numThreads; ++t) {
        size_t begin = t * chunkSize;
        size_t end = std::min(n, begin + chunkSize);
        if (begin >= end) break;
        futures.push_back(std::async(std::launch::async, buildPartial, std::cref(docs), begin, end));
    }

    for (auto& f : futures) {
        merge(f.get());
    }
}

const InvertedIndex::PostingList* InvertedIndex::getPostings(const std::string& term) const {
    auto it = index_.find(term);
    return it == index_.end() ? nullptr : &it->second;
}

int InvertedIndex::documentFrequency(const std::string& term) const {
    const PostingList* postings = getPostings(term);
    return postings ? static_cast<int>(postings->size()) : 0;
}

std::vector<std::string> InvertedIndex::vocabulary() const {
    std::vector<std::string> words;
    words.reserve(index_.size());
    for (const auto& [term, _] : index_) {
        words.push_back(term);
    }
    return words;
}

} // namespace searchx
