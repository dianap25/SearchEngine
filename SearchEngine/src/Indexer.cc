// Authors: Alesia Filinkova, Diana Pelin
// Description: Implementation of Indexer. Tokenizes extracted text
// into normalized terms with positions and hands the resulting
// vector to Repository::saveTermPositionsBatch.


#include "Indexer.h"

#include <cctype>
#include <iostream>
#include <sstream>

Indexer::Indexer(Repository& repository)
    : repository_(repository) {
}

bool Indexer::indexFile(int file_id, const std::string& content) {
    std::vector<std::pair<std::string, int>> tokens = tokenize(content);

    if (!repository_.deleteIndexForFile(file_id)) {
        return false;
    }

    return repository_.saveTermPositionsBatch(file_id, tokens);
}

std::vector<std::pair<std::string, int>> Indexer::tokenize(const std::string& content) const {
    std::vector<std::pair<std::string, int>> tokens;

    std::stringstream stream(content);
    std::string raw_token;
    int position = 0;

    while (stream >> raw_token) {
        std::string normalized = normalizeToken(raw_token);

        if (!normalized.empty()) {
            tokens.emplace_back(normalized, position);
            ++position;
        }
    }

    return tokens;
}

std::string Indexer::normalizeToken(const std::string& token) const {
    std::string normalized;

    for (unsigned char character : token) {
        if (std::isalnum(character)) {
            normalized.push_back(static_cast<char>(std::tolower(character)));
        }
    }

    return normalized;
}
