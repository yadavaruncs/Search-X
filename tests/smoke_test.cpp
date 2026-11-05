// Plain assert()-based sanity checks for each core component -- no test
// framework, just enough to catch a broken build before it hits a demo.
#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
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

    // ids are labels, not positions
    InvertedIndex sparse;
    sparse.build({{100, "first"}, {7, "second"}});
    assert(sparse.getDocument(100).text == "first");
    assert(sparse.getDocument(7).text == "second");

    bool threw = false;
    try {
        sparse.getDocument(0); // valid position, but no document has this id
    } catch (const std::out_of_range&) {
        threw = true;
    }
    assert(threw);
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
    assert(TopK::select(scored, 0).empty());
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

    LRUCache disabled(0);
    disabled.put("a", a);
    assert(!disabled.get("a", out));
    std::cout << "  LRUCache: OK\n";
}

// loadDocuments is private to main.cpp and benchmark.cpp, so run the real
// binaries from the build directory, which has no data/docs.txt.
static void testMissingDocsFile(const std::filesystem::path& binDir) {
    namespace fs = std::filesystem;
    const fs::path startDir = fs::current_path();
    fs::current_path(binDir);
    assert(!fs::exists("data/docs.txt"));

    for (const std::string name : {"searchx", "benchmark"}) {
        const std::string exe = (fs::path(".") / name).make_preferred().string();
        const std::string outFile = name + ".stdout.txt";
        const std::string errFile = name + ".stderr.txt";
        // "quit" ends searchx's prompt loop if a missing file stops being fatal.
        const std::string command = "echo quit | " + exe + " > " + outFile + " 2> " + errFile;
        assert(std::system(command.c_str()) != 0);

        std::ifstream err(errFile);
        std::string message;
        std::getline(err, message);
        err.close();
        assert(message == "cannot open data/docs.txt");
        fs::remove(outFile);
        fs::remove(errFile);
    }

    fs::current_path(startDir);
    std::cout << "  Missing docs file: OK\n";
}

int main(int, char** argv) {
    std::cout << "Running SearchX smoke tests...\n";
    testTokenizer();
    testInvertedIndex();
    testRankerAndTopK();
    testTrie();
    testLRUCache();
    testMissingDocsFile(std::filesystem::absolute(argv[0]).parent_path());
    std::cout << "All smoke tests passed.\n";
    return 0;
}
