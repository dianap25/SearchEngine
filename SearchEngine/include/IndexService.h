// Authors: Alesia Filinkova, Diana Pelin
// Description: Wires Scanner, Extractor, Repository and Indexer
// together behind a single indexDirectory() entry point used by the
// CLI "index" command.

#pragma once

#include "Database.h"
#include "IndexSummary.h"

#include <string>

/**
 * @brief Indexes every supported file in a directory using a
 *        "transaction per file" strategy.
 */
class IndexService {
public:
    explicit IndexService(Database& database);

    /**
     * @brief Walks @p root_path, extracts content and indexes
     *        everything.
     * @param root_path Directory to scan recursively.
     * @return Counters describing the run (scanned, indexed, skipped,
     *         failed).
     */
    IndexSummary indexDirectory(const std::string& root_path);

private:
    Database& database_;
};
