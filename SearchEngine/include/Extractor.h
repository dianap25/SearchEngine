// Authors: Alesia Filinkova, Diana Pelin
// Description: Abstract base of the extractor hierarchy (Strategy
// pattern). Concrete subclasses (TextExtractor, PdfExtractor) plug
// into a single dispatcher (ExtractorFactory) that picks the right
// strategy based on the file extension.

#pragma once

#include "ExtractResult.h"

#include <string>

/**
 * @brief Strategy interface for reading textual content from a file.
 */
class Extractor {
public:
    virtual ~Extractor() = default;

    /**
     * @brief Extracts textual content from @p file_path.
     * @param file_path Path to the file on disk.
     * @return Extracted content or a failure result.
     */
    virtual ExtractResult extract(const std::string& file_path) const = 0;
};
