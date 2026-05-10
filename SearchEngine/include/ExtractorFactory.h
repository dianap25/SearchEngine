// Authors: Alesia Filinkova, Diana Pelin
// Description: Factory that maps a file path (by extension) to the
// concrete Extractor strategy that knows how to read it.

#pragma once

#include "Extractor.h"

#include <memory>
#include <string>

/**
 * @brief Picks the right extractor strategy for a given file path.
 */
class ExtractorFactory {
public:
    /**
     * @brief Build an extractor based on the path's extension.
     * @param file_path Path to the file on disk. The file does not
     *                  need to exist; only the extension is used.
     * @return Owning pointer to a concrete Extractor. PDF files
     *         return a PdfExtractor; everything else (.txt, .tex,
     *         no extension) returns a TextExtractor.
     */
    static std::unique_ptr<Extractor> create(const std::string& file_path);
};
