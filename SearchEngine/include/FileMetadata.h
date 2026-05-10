// Authors: Alesia Filinkova, Diana Pelin
// Description: Data structure describing a single file on disk plus
// the state the index keeps about that file (database id, content
// hash).

#pragma once

#include <cstdint>
#include <string>

/**
 * @brief Snapshot of a file as it exists on disk and in the index.
 *
 * Scanner fills the disk-derived fields (path, name, extension, size,
 * modified_time). Repository fills @c id and @c content_hash from the
 * database. RefreshEngine compares hashes to decide whether a file
 * needs to be re-indexed.
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
