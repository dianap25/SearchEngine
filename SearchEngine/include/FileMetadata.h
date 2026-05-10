//Alesia Filinkova
//Diana Pelin

#pragma once

#include <cstdint>
#include <string>

struct FileMetadata {
    int id = 0;
    std::string path;
    std::string name;
    std::string extension;
    std::uintmax_t size = 0;
    std::int64_t modifiedTime = 0;
    std::string contentHash;
};