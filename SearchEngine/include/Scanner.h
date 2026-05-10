// Authors: Alesia Filinkova, Diana Pelin
// Description: Recursive directory scanner. Walks the directory tree
// and builds FileMetadata records for files with supported extensions
// (.txt, .tex, .pdf, no extension).

#pragma once

#include "FileMetadata.h"

#include <filesystem>
#include <string>
#include <vector>

/**
 * @brief Walks a directory tree and emits FileMetadata for every
 *        file that can be indexed.
 */
class Scanner {
public:
    /**
     * @brief Scans a directory recursively.
     * @param root_path Top-level directory to scan.
     * @return Metadata of every supported regular file.
     */
    std::vector<FileMetadata> scan(const std::string& root_path);

private:
    bool isSupported(const std::filesystem::path& path) const;
    FileMetadata buildMetadata(const std::filesystem::directory_entry& entry) const;
    std::int64_t toUnixTimestamp(const std::filesystem::file_time_type& file_time) const;
};
