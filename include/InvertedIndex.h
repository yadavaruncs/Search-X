#pragma once
#include <string>
#include <unordered_map>
#include <vector>

namespace searchx {

struct Document {
    int id;
    std::string text;
};

// Maps each word -> (docId -> how many times it appears in that doc).
// A hash map gives O(1) average lookup for "which docs contain this word",
// which is the whole point of an inverted index vs. scanning every document.
class InvertedIndex {
public:
    using PostingList = std::unordered_map<int, int>; // docId -> term frequency

    // Builds the index from scratch, splitting the documents across
    // numThreads workers and merging their partial indexes.
    void build(const std::vector<Document>& docs, unsigned numThreads = 4);

    const PostingList* getPostings(const std::string& term) const;
    int documentFrequency(const std::string& term) const; // # of docs containing term
    int documentCount() const { return static_cast<int>(documents_.size()); }
    const Document& getDocument(int id) const { return documents_.at(id); }
    const std::vector<Document>& documents() const { return documents_; }
    std::vector<std::string> vocabulary() const;

private:
    std::vector<Document> documents_;
    std::unordered_map<std::string, PostingList> index_;

    static std::unordered_map<std::string, PostingList> buildPartial(
        const std::vector<Document>& docs, size_t begin, size_t end);
    void merge(std::unordered_map<std::string, PostingList>&& partial);
};

} // namespace searchx
