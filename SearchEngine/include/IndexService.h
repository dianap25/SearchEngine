// Authors: Alesia Filinkova, Diana Pelin
// Description: High-level orchestration of a single index run. Wires
// together Scanner, Extractor, Repository and Indexer behind a
// single indexDirectory() entry point used by the index CLI command.

#pragma once

#include "Database.h"
#include "IndexSummary.h"

#include <string>

/**
 * @brief Indexes every supported file under a directory in one
 *        transaction-per-file pass.
 */
class IndexService {
public:
    explicit IndexService(Database& database);

    /**
     * @brief Walk @p root_path, extract content, and index everything.
     * @param root_path Directory to scan recursively.
     * @return Counters describing the run (scanned, indexed, skipped, failed).
     */
    IndexSummary indexDirectory(const std::string& root_path);

private:
    Database& database_;
};
