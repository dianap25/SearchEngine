// Authors: Alesia Filinkova, Diana Pelin
// Description: Implementation of PdfExtractor. Section 4 keeps the
// existing popen-based call to pdftotext; Section 5 swaps it for a
// fork+execvp implementation that never invokes a shell.

#include "PdfExtractor.h"

#include <cstdio>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

ExtractResult PdfExtractor::extract(const std::string& file_path) const {
    if (!fs::exists(file_path)) {
        return ExtractResult::fail("File does not exist");
    }

    std::string command = "pdftotext '" + file_path + "' -";
    std::string result;
    char buffer[256];

    FILE* pipe = popen(command.c_str(), "r");
    if (pipe == nullptr) {
        return ExtractResult::fail("pdftotext failed to start");
    }

    while (std::fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    pclose(pipe);

    if (result.empty()) {
        return ExtractResult::fail("PDF extraction returned empty content");
    }

    return ExtractResult::ok(result);
}
