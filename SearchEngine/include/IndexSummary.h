// Authors: Alesia Filinkova, Diana Pelin
// Description: Counters returned by IndexService::indexDirectory so the
// CLI can report how many files were scanned, indexed, skipped or
// failed during a single indexing run.

#pragma once

/**
 * @brief Aggregate counts collected over one indexing run.
 */
struct IndexSummary {
    int scanned_files = 0;
    int indexed_files = 0;
    int skipped_files = 0;
    int failed_files = 0;
};
