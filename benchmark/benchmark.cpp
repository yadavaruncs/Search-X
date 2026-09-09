// Compares four ways of finding documents that contain a word, to show why
// each data structure choice in SearchX matters:
//   1. Linear scan   -- re-tokenize and check every document, every query.  O(total corpus size)
//   2. Hash lookup    -- InvertedIndex postings lookup, no ranking.         O(1) average
//   3. Inverted+TF-IDF-- full ranked search (index lookup + score + top-K). O(matches * log k)
//   4. Inverted+cache -- same ranked search, but repeat queries hit the LRU cache. O(1) on a hit
#include <chrono>
#include <fstream>
#include <iostream>
#include <vector>
#include "InvertedIndex.h"
#include "LRUCache.h"
#include "Ranker.h"
#include "Tokenizer.h"
#include "TopK.h"

using namespace searchx;
using Clock = std::chrono::steady_clock;

static std::vector<Document> loadDocuments(const std::string& path) {
    std::ifstream file(path);
    std::vector<Document> docs;
    std::string line;
    int id = 0;
    while (std::getline(file, line)) {
        if (!line.empty()) docs.push_back({id++, line});
    }
    return docs;
}

static double elapsedMs(Clock::time_point a, Clock::time_point b) {
    return std::chrono::duration_cast<std::chrono::microseconds>(b - a).count() / 1000.0;
}

// Naive baseline: no index at all, tokenize + scan every document per query.
static int linearSearch(const std::vector<Document>& docs, const std::string& term) {
    int matches = 0;
    for (const Document& doc : docs) {
        for (const std::string& word : Tokenizer::tokenize(doc.text)) {
            if (word == term) {
                ++matches;
                break;
            }
        }
    }
    return matches;
}

int main() {
    auto docs = loadDocuments("data/docs.txt");
    std::cout << "Benchmarking on " << docs.size() << " documents\n\n";

    InvertedIndex index;
    index.build(docs);
    Ranker ranker(index);

    const std::vector<std::string> queries = {
        "cache", "search", "career", "inverted", "algorithm", "data", "index", "python"
    };
    const int repeats = 20; // run each query several times so timings are stable

    // 1. Linear scan
    auto t0 = Clock::now();
    for (int r = 0; r < repeats; ++r) {
        for (const std::string& q : queries) linearSearch(docs, q);
    }
    double linearMs = elapsedMs(t0, Clock::now());

    // 2. Hash lookup only (no ranking) -- isolates the win from the inverted index itself
    auto t1 = Clock::now();
    for (int r = 0; r < repeats; ++r) {
        for (const std::string& q : queries) index.getPostings(q);
    }
    double hashMs = elapsedMs(t1, Clock::now());

    // 3. Full ranked search: inverted index + TF-IDF + top-K heap
    auto t2 = Clock::now();
    for (int r = 0; r < repeats; ++r) {
        for (const std::string& q : queries) {
            auto scored = ranker.score(Tokenizer::tokenize(q));
            TopK::select(scored, 5);
        }
    }
    double rankedMs = elapsedMs(t2, Clock::now());

    // 4. Same ranked search, but wrapped in an LRU cache (queries repeat, so
    //    after the first pass every lookup is a cache hit).
    LRUCache cache(50);
    auto t3 = Clock::now();
    for (int r = 0; r < repeats; ++r) {
        for (const std::string& q : queries) {
            std::vector<ScoredDoc> results;
            if (!cache.get(q, results)) {
                auto scored = ranker.score(Tokenizer::tokenize(q));
                results = TopK::select(scored, 5);
                cache.put(q, results);
            }
        }
    }
    double cachedMs = elapsedMs(t3, Clock::now());

    int totalQueries = repeats * static_cast<int>(queries.size());
    std::cout << "Each method ran " << totalQueries << " queries (" << queries.size()
              << " distinct terms x " << repeats << " repeats)\n\n";
    std::cout << "1. Linear scan (no index):         " << linearMs << " ms total, "
              << (linearMs * 1000.0 / totalQueries) << " us/query\n";
    std::cout << "2. Hash lookup (index, no rank):    " << hashMs << " ms total, "
              << (hashMs * 1000.0 / totalQueries) << " us/query\n";
    std::cout << "3. Inverted index + TF-IDF + top-K: " << rankedMs << " ms total, "
              << (rankedMs * 1000.0 / totalQueries) << " us/query\n";
    std::cout << "4. Inverted index + LRU cache:       " << cachedMs << " ms total, "
              << (cachedMs * 1000.0 / totalQueries) << " us/query\n\n";
    std::cout << "Speedup, linear scan -> ranked search: " << (linearMs / rankedMs) << "x\n";
    std::cout << "Speedup, ranked search -> cached:      " << (rankedMs / cachedMs) << "x\n";
    return 0;
}
