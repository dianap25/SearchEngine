// Authors: Alesia Filinkova, Diana Pelin
// Description: Incremental updater. Walks the filesystem, hashes each
// file and synchronizes the index with the current state on disk.

#pragma once

#include "Database.h"

#include <string>

/**
 * @brief Synchronizes an existing index with the current state on
 *        disk.
 */
class RefreshEngine {
public:
    /**
     * @brief Synchronizes the file tree at @p root_path with the
     *        index stored in @p db.
     * @param root_path Top-level directory previously indexed.
     * @param db Database holding the index. The schema is initialized
     *           on the first call, so refresh works even without a
     *           prior "index" command.
     */
    void refresh(const std::string& root_path, Database& db);
};
