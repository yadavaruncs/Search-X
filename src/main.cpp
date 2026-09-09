#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>
#include "InvertedIndex.h"
#include "LRUCache.h"
#include "Ranker.h"
#include "Tokenizer.h"
#include "TopK.h"
#include "Trie.h"

using namespace searchx;
using Clock = std::chrono::steady_clock;

static std::vector<Document> loadDocuments(const std::string& path) {
    std::ifstream file(path);
    std::vector<Document> docs;
    std::string line;
    int id = 0;
    while (std::getline(file, line)) {
        if (!line.empty()) {
            docs.push_back({id++, line});
        }
    }
    return docs;
}

static void printResults(const InvertedIndex& index, const std::vector<ScoredDoc>& results, size_t limit) {
    if (results.empty()) {
        std::cout << "  (no matches)\n";
        return;
    }
    size_t shown = 0;
    for (const ScoredDoc& r : results) {
        if (shown++ >= limit) break;
        std::cout << "  [" << r.score << "] doc#" << r.docId << ": "
                   << index.getDocument(r.docId).text << "\n";
    }
}

int main() {
    std::cout << "=======================================\n";
    std::cout << "  SearchX -- mini search engine (C++20)\n";
    std::cout << "=======================================\n\n";

    auto docs = loadDocuments("data/docs.txt");
    std::cout << "Loaded " << docs.size() << " documents\n";

    // Single-threaded vs multithreaded index build, side by side, so the
    // speedup from parallelizing across document chunks is visible.
    InvertedIndex singleThreaded;
    auto t0 = Clock::now();
    singleThreaded.build(docs, 1);
    auto t1 = Clock::now();

    unsigned hwThreads = std::max(2u, std::thread::hardware_concurrency());
    InvertedIndex index;
    auto t2 = Clock::now();
    index.build(docs, hwThreads);
    auto t3 = Clock::now();

    auto ms = [](Clock::time_point a, Clock::time_point b) {
        return std::chrono::duration_cast<std::chrono::microseconds>(b - a).count() / 1000.0;
    };
    std::cout << "Index build (1 thread):  " << ms(t0, t1) << " ms\n";
    std::cout << "Index build (" << hwThreads << " threads): " << ms(t2, t3) << " ms\n";
    std::cout << "Vocabulary size: " << index.vocabulary().size() << " words\n\n";

    Ranker ranker(index);
    const size_t kResults = 5;

    // Every indexed word also goes into a Trie so prefix lookups ("car" ->
    // care, career, careful...) don't need to scan the whole vocabulary.
    Trie trie;
    for (const std::string& word : index.vocabulary()) {
        trie.insert(word);
    }

    // Caches a query string -> its already-ranked top-K, so repeating a
    // search skips scoring + heap selection entirely.
    LRUCache cache(100);

    std::cout << "Commands: 'auto <prefix>' for autocomplete, 'stats' for cache stats, "
                 "anything else searches. 'quit' to exit.\n";
    std::string line;
    while (true) {
        std::cout << "\nsearch> ";
        if (!std::getline(std::cin, line) || line == "quit") break;
        if (line.empty()) continue;

        if (line.rfind("auto ", 0) == 0) {
            std::string prefix = line.substr(5);
            std::vector<std::string> suggestions = trie.autocomplete(prefix);
            if (suggestions.empty()) {
                std::cout << "  (no suggestions)\n";
            } else {
                for (const std::string& s : suggestions) std::cout << "  " << s << "\n";
            }
            continue;
        }

        if (line == "stats") {
            std::cout << "  cache hits: " << cache.hits() << ", misses: " << cache.misses() << "\n";
            continue;
        }

        auto queryStart = Clock::now();
        std::vector<ScoredDoc> results;
        bool cached = cache.get(line, results);

        if (!cached) {
            std::vector<std::string> queryTerms = Tokenizer::tokenize(line);
            std::vector<ScoredDoc> scored = ranker.score(queryTerms);

            // Min-heap top-K instead of sorting every match: O(n log k) not
            // O(n log n), and k (here 5) stays tiny even as matches grow.
            results = TopK::select(scored, kResults);
            cache.put(line, results);
        }
        auto queryEnd = Clock::now();

        std::cout << "top " << kResults << (cached ? " [cache hit]" : " [computed]")
                   << " (" << ms(queryStart, queryEnd) << " ms):\n";
        printResults(index, results, kResults);
    }
    return 0;
}
