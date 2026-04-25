//Alesia Filinkova
//Diana Pelin

#pragma once

#include <cstdint>
#include <string>

struct FileMetadata {
    std::string path;
    std::string name;
    std::string extension;
    std::uintmax_t size;
    std::int64_t modifiedTime;
};