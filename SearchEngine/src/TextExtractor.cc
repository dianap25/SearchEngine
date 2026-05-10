// Autorzy: Alesia Filinkova, Diana Pelin


#include "TextExtractor.h"

#include <filesystem>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

ExtractResult TextExtractor::extract(const std::string& file_path) const {
    if (!fs::exists(file_path)) {
        return ExtractResult::fail("File does not exist");
    }

    std::ifstream file(file_path);

    if (!file.is_open()) {
        return ExtractResult::fail("Cannot open text file");
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    std::string content = buffer.str();

    if (content.empty()) {
        return ExtractResult::fail("Empty text file");
    }

    return ExtractResult::ok(content);
}
