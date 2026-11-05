#include "DocumentLoader.h"
#include <cstdlib>
#include <fstream>
#include <iostream>

namespace searchx {

std::vector<Document> loadDocuments(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        std::cerr << "cannot open " << path << "\n";
        std::exit(1);
    }
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

} // namespace searchx
