#pragma once
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace searchx {

// Prefix tree over the vocabulary, used for autocomplete. Looking up a
// prefix costs O(length of prefix) to walk down to the right node, then a
// small DFS to collect matching words -- independent of how many total
// words are stored, unlike scanning the whole vocabulary for matches.
class Trie {
public:
    void insert(const std::string& word);
    std::vector<std::string> autocomplete(const std::string& prefix, size_t maxResults = 5) const;

private:
    struct Node {
        std::map<char, std::unique_ptr<Node>> children; // std::map keeps children sorted -> alphabetical suggestions
        bool isWord = false;
    };

    Node root_;

    void collect(const Node* node, std::string& prefix, std::vector<std::string>& results, size_t maxResults) const;
};

} // namespace searchx
