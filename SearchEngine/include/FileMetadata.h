// Authors: Alesia Filinkova, Diana Pelin
// Description: Plain data describing a single file on disk plus the
// derived state that the index keeps about it (database id, content
// hash). Used as the shared exchange type between Scanner, Repository,
// Indexer and RefreshEngine.

#pragma once

#include <cstdint>
#include <string>

/**
 * @brief Snapshot of a file as it appears on disk and in the index.
 *
 * Scanner fills the on-disk fields (path, name, extension, size,
 * modified_time). Repository fills @c id and @c content_hash from the
 * database. RefreshEngine compares hashes to decide whether to
 * re-index.
 */
struct FileMetadata {
    int id = 0;
    std::string path;
    std::string name;
    std::string extension;
    std::uintmax_t size = 0;
    std::int64_t modified_time = 0;
    std::string content_hash;
};
