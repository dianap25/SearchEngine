//Alesia Filinkova
//Diana Pelin

#pragma once
#include "ExtractResult.h"
#include <string>

class Extractor {
public:
    ExtractResult extract(const std::string& filePath);
private:
    ExtractResult extractText(const std::string& filePath);
    ExtractResult extractPdf(const std::string& filePath);
    std::string extractPdfInternal(const std::string& filePath);
};