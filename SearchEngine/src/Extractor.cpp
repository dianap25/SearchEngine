#include "Extractor.h"

#include <fstream>
#include <sstream>

std::string Extractor::extractTextFile(const std::string& path) {
    std::ifstream input(path);
    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}