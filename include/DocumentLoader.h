#pragma once
#include <string>
#include <vector>
#include "InvertedIndex.h"

namespace searchx {

// Reads one document per non-empty line, numbering ids from 0. Every caller
// is a CLI that can't run without its data, so a file that won't open prints
// the path to stderr and exits with status 1.
std::vector<Document> loadDocuments(const std::string& path);

} // namespace searchx
