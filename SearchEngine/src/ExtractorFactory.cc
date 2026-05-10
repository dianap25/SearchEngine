// Authors: Alesia Filinkova, Diana Pelin
// Description: Implementation of ExtractorFactory: returns a
// PdfExtractor for .pdf paths and a TextExtractor for everything
// else.


#include "ExtractorFactory.h"

#include "PdfExtractor.h"
#include "TextExtractor.h"

#include <filesystem>

std::unique_ptr<Extractor> ExtractorFactory::create(const std::string& file_path) {
    std::filesystem::path path(file_path);
    std::string extension = path.extension().string();

    if (extension == ".pdf") {
        return std::make_unique<PdfExtractor>();
    }

    return std::make_unique<TextExtractor>();
}
