// Authors: Alesia Filinkova, Diana Pelin
// Description: Implementation of Extractor. Dispatches PDF files to
// pdftotext and treats every other supported extension as a plain
// text file read directly from disk.

#include "Extractor.h"

#include <filesystem>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

ExtractResult Extractor::extract(const std::string& file_path) {
    if (!fs::exists(file_path)) {
        return ExtractResult::fail("File does not exist");
    }

    if (file_path.size() >= 4 &&
        file_path.substr(file_path.size() - 4) == ".pdf") {
        return extractPdf(file_path);
    }

    return extractText(file_path);
}

ExtractResult Extractor::extractText(const std::string& file_path) {
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

std::string Extractor::extractPdfInternal(const std::string& file_path) {
    std::string command = "pdftotext '" + file_path + "' -";
    std::string result;
    char buffer[256];

    FILE* pipe = popen(command.c_str(), "r");
    if (pipe == nullptr) return "";

    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    pclose(pipe);
    return result;
}

ExtractResult Extractor::extractPdf(const std::string& file_path) {
    std::string result = extractPdfInternal(file_path);

    if (result.empty()) {
        return ExtractResult::fail("PDF extraction returned empty content");
    }

    return ExtractResult::ok(result);
}
