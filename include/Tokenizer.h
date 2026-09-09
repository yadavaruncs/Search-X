#pragma once
#include <string>
#include <vector>

namespace searchx {

// Splits raw text into lowercase alphanumeric words, dropping punctuation.
class Tokenizer {
public:
    static std::vector<std::string> tokenize(const std::string& text);
};

} // namespace searchx
