// Authors: Alesia Filinkova, Diana Pelin
// Description: Implementation of ExtractorFactory. Inspects the
// file extension and returns the matching concrete Extractor; defaults
// to TextExtractor for unknown extensions.

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
