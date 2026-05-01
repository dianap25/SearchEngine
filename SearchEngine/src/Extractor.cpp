//Alesia Filinkova
//Diana Pelin

#include "Extractor.h"
#include <fstream>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

ExtractResult Extractor::extract(const std::string& filePath) {
    if (!fs::exists(filePath)) {
        return ExtractResult::fail("File does not exist");
    }

    if (filePath.size() >= 4 &&
        filePath.substr(filePath.size() - 4) == ".pdf") {

        return extractPdf(filePath);
    }

    return extractText(filePath);
}

ExtractResult Extractor::extractText(const std::string& filePath) {
    std::ifstream file(filePath);

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

std::string Extractor::extractPdfInternal(const std::string& filePath) {
    std::string command = "pdftotext '" + filePath + "' -";
    std::string result;
    char buffer[256];

    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) return "";

    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    pclose(pipe);
    return result;
}

ExtractResult Extractor::extractPdf(const std::string& filePath) {
    std::string result = extractPdfInternal(filePath);

    if (result.empty()) {
        return ExtractResult::fail("PDF extraction returned empty content");
    }

    return ExtractResult::ok(result);
}
