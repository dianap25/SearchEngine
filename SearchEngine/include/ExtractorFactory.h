// Authors: Alesia Filinkova, Diana Pelin
// Description: Factory that maps a file path (by its extension) to
// the concrete extraction strategy able to read it.

#pragma once

#include "Extractor.h"

#include <memory>
#include <string>

/**
 * @brief Picks the appropriate extraction strategy for a given file
 *        path.
 */
class ExtractorFactory {
public:
    /**
     * @brief Builds an extractor based on the path extension.
     * @param file_path Path to a file on disk. The file does not have
     *                  to exist; only the extension is considered.
     * @return Owning pointer to a concrete extractor. PDF files get a
     *         PdfExtractor; everything else (.txt, .tex, no
     *         extension) gets a TextExtractor.
     */
    static std::unique_ptr<Extractor> create(const std::string& file_path);
};
