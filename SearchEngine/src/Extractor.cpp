#include "Extractor.h"
#include <fstream>
#include <sstream>
#include <cstdlib>

std::string Extractor::extract(const std::string& filePath) {
    if (filePath.size() >= 4 && filePath.substr(filePath.size() - 4) == ".pdf") {
        return extractPdf(filePath);
    }
    return extractText(filePath);
}

std::string Extractor::extractText(const std::string& filePath) {
    std::ifstream file(filePath);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string Extractor::extractPdf(const std::string& filePath) {
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
