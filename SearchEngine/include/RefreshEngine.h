// Authors: Alesia Filinkova, Diana Pelin
// Description: Incremental updater that walks the filesystem, hashes
// each file and reconciles the index: new files are indexed, changed
// files are re-indexed, and files that disappeared from disk are
// removed from the database.

#pragma once

#include "Database.h"

#include <string>

/**
 * @brief Brings an already-built index back in sync with the disk.
 */
class RefreshEngine {
public:
    /**
     * @brief Reconcile the on-disk tree at @p root_path with the index
     *        in @p db.
     * @param root_path Top-level directory previously indexed.
     * @param db Database holding the index. The schema is initialized
     *           on the first call so refresh works without a prior
     *           index command.
     */
    void refresh(const std::string& root_path, Database& db);
};
