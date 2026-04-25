//Alesia Filinkova
//Diana Pelin

#pragma once

#include "Repository.h"

#include <string>
#include <utility>
#include <vector>

class Indexer {
public:
    explicit Indexer(Repository& repository);

    bool indexFile(int fileId, const std::string& content);

    std::vector<std::pair<std::string, int>> tokenize(const std::string& content) const;

private:
    std::string normalizeToken(const std::string& token) const;

    Repository& repository_;
};