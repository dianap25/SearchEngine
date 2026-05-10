// Authors: Alesia Filinkova, Diana Pelin
// Description: Tokenizer + posting writer. Splits extracted text into
// normalized terms with their positions and stores them via Repository
// so search-content can later look them up.

#pragma once

#include "Repository.h"

#include <string>
#include <utility>
#include <vector>

/**
 * @brief Builds the inverted index for a single file.
 *
 * Holds a reference to the Repository it writes through; lifetime of
 * the Repository must outlive the Indexer.
 */
class Indexer {
public:
    explicit Indexer(Repository& repository);

    /**
     * @brief Tokenize @p content and write all postings for the file.
     * @param file_id Database id of the file.
     * @param content Extracted text of the file.
     * @return true when every posting was written successfully.
     */
    bool indexFile(int file_id, const std::string& content);

    /**
     * @brief Tokenize text into (normalized term, position) pairs.
     * @param content Raw extracted text.
     */
    std::vector<std::pair<std::string, int>> tokenize(const std::string& content) const;

private:
    std::string normalizeToken(const std::string& token) const;

    Repository& repository_;
};
