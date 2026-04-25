//Alesia Filinkova
//Diana Pelin

#pragma once

#include "FileMetadata.h"

#include <filesystem>
#include <string>
#include <vector>

class Scanner {
public:
    std::vector<FileMetadata> scan(const std::string& rootPath);

private:
    bool isSupported(const std::filesystem::path& path) const;
    FileMetadata buildMetadata(const std::filesystem::directory_entry& entry) const;
    std::int64_t toUnixTimestamp(const std::filesystem::file_time_type& fileTime) const;
};