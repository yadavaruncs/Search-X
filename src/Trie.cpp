#include "Trie.h"

namespace searchx {

void Trie::insert(const std::string& word) {
    Node* node = &root_;
    for (char c : word) {
        auto& child = node->children[c];
        if (!child) {
            child = std::make_unique<Node>();
        }
        node = child.get();
    }
    node->isWord = true;
}

void Trie::collect(const Node* node, std::string& prefix, std::vector<std::string>& results, size_t maxResults) const {
    if (results.size() >= maxResults) return;
    if (node->isWord) {
        results.push_back(prefix);
    }
    for (const auto& [ch, child] : node->children) {
        if (results.size() >= maxResults) return;
        prefix.push_back(ch);
        collect(child.get(), prefix, results, maxResults);
        prefix.pop_back();
    }
}

std::vector<std::string> Trie::autocomplete(const std::string& prefix, size_t maxResults) const {
    const Node* node = &root_;
    for (char c : prefix) {
        auto it = node->children.find(c);
        if (it == node->children.end()) {
            return {}; // nothing in the vocabulary starts with this prefix
        }
        node = it->second.get();
    }

    std::vector<std::string> results;
    std::string current = prefix;
    collect(node, current, results, maxResults);
    return results;
}

} // namespace searchx
