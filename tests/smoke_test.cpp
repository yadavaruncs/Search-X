// Plain assert()-based sanity checks for each core component -- no test
// framework, just enough to catch a broken build before it hits a demo.
#include <cassert>
#include <iostream>
#include "InvertedIndex.h"
#include "LRUCache.h"
#include "Ranker.h"
#include "Tokenizer.h"
#include "Trie.h"
#include "TopK.h"

using namespace searchx;

static void testTokenizer() {
    auto tokens = Tokenizer::tokenize("Hello, World!  C++20.");
    assert(tokens.size() == 4);
    assert(tokens[0] == "hello");
    assert(tokens[1] == "world");
    assert(tokens[2] == "c");
    assert(tokens[3] == "20");
    std::cout << "  Tokenizer: OK\n";
}

static void testInvertedIndex() {
    std::vector<Document> docs = {
        {0, "the cat sat on the mat"},
        {1, "the dog sat"},
        {2, "cats and dogs"},
    };
    InvertedIndex index;
    index.build(docs, 2); // exercise the multithreaded path with a tiny corpus

    const auto* sat = index.getPostings("sat");
    assert(sat != nullptr);
    assert(sat->size() == 2);           // "sat" appears in doc 0 and doc 1
    assert(sat->at(0) == 1);            // once in doc 0
    assert(index.getPostings("xyz") == nullptr); // absent term
    assert(index.documentFrequency("the") == 2);
    std::cout << "  InvertedIndex: OK\n";
}

static void testRankerAndTopK() {
    std::vector<Document> docs = {
        {0, "cache cache cache"},   // mentions "cache" 3x
        {1, "cache once"},          // mentions "cache" 1x
        {2, "unrelated text"},      // no match
    };
    InvertedIndex index;
    index.build(docs);
    Ranker ranker(index);

    auto scored = ranker.score({"cache"});
    assert(scored.size() == 2); // doc 2 never scored -- no match at all

    auto top = TopK::select(scored, 1);
    assert(top.size() == 1);
    assert(top[0].docId == 0); // doc 0 mentions "cache" more -> higher TF-IDF
    std::cout << "  Ranker + TopK: OK\n";
}

static void testTrie() {
    Trie trie;
    for (const std::string& w : {"car", "care", "career", "cat", "dog"}) {
        trie.insert(w);
    }
    auto results = trie.autocomplete("car");
    assert(results.size() == 3); // car, care, career
    assert(trie.autocomplete("zzz").empty());
    std::cout << "  Trie: OK\n";
}

static void testLRUCache() {
    LRUCache cache(2); // capacity 2
    std::vector<ScoredDoc> a = {{1, 1.0}};
    std::vector<ScoredDoc> b = {{2, 2.0}};
    std::vector<ScoredDoc> c = {{3, 3.0}};

    cache.put("a", a);
    cache.put("b", b);
    cache.put("c", c); // evicts "a" (least recently used)

    std::vector<ScoredDoc> out;
    assert(!cache.get("a", out)); // evicted
    assert(cache.get("b", out) && out[0].docId == 2);
    assert(cache.get("c", out) && out[0].docId == 3);
    std::cout << "  LRUCache: OK\n";
}

int main() {
    std::cout << "Running SearchX smoke tests...\n";
    testTokenizer();
    testInvertedIndex();
    testRankerAndTopK();
    testTrie();
    testLRUCache();
    std::cout << "All smoke tests passed.\n";
    return 0;
}
