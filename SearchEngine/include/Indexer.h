// Authors: Alesia Filinkova, Diana Pelin
// Description: Splits extracted text into normalized terms with their
// positions and writes them through Repository, so that search-content
// can find them later.

#pragma once

#include "Repository.h"

#include <string>
#include <utility>
#include <vector>

/**
 * @brief Builds the inverted index for a single file.
 *
 * Holds a reference to the Repository through which it persists data;
 * the Repository object must outlive the Indexer.
 */
class Indexer {
public:
    explicit Indexer(Repository& repository);

    /**
     * @brief Tokenizes @p content and writes every posting for the
     *        file.
     * @param file_id Database id of the file.
     * @param content Extracted text of the file.
     * @return true if every posting was persisted successfully.
     */
    bool indexFile(int file_id, const std::string& content);

    /**
     * @brief Splits text into (normalized term, position) pairs.
     * @param content Raw extracted text.
     */
    std::vector<std::pair<std::string, int>> tokenize(const std::string& content) const;

private:
    std::string normalizeToken(const std::string& token) const;

    Repository& repository_;
};
