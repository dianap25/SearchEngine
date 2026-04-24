#pragma once
#include <string>

class Extractor {
public:
    std::string extract(const std::string& filePath);
private:
    std::string extractText(const std::string& filePath);
    std::string extractPdf(const std::string& filePath);
};