//Alesia Filinkova
//Diana Pelin

#include "Indexer.h"

#include <cctype>
#include <iostream>
#include <sstream>

Indexer::Indexer(Repository& repository)
    : repository_(repository) {
}

bool Indexer::indexFile(int fileId, const std::string& content) {
    std::vector<std::pair<std::string, int>> tokens = tokenize(content);

    if (!repository_.deleteIndexForFile(fileId)) {
        return false;
    }

    for (const auto& [term, position] : tokens) {
        if (!repository_.saveTermPosition(fileId, term, position)) {
            std::cerr << "Failed to save token: " << term << "\n";
            return false;
        }
    }

    return true;
}

std::vector<std::pair<std::string, int>> Indexer::tokenize(const std::string& content) const {
    std::vector<std::pair<std::string, int>> tokens;

    std::stringstream stream(content);
    std::string rawToken;
    int position = 0;

    while (stream >> rawToken) {
        std::string normalized = normalizeToken(rawToken);

        if (!normalized.empty()) {
            tokens.emplace_back(normalized, position);
            position++;
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