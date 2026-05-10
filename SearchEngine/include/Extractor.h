// Authors: Alesia Filinkova, Diana Pelin
// Description: Abstract base of the text-extractor strategy
// hierarchy. Concrete subclasses (TextExtractor, PdfExtractor) plug
// into a single dispatcher (ExtractorFactory) which picks the right
// strategy based on the file's extension.

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
     * @brief Extract textual content from @p file_path.
     * @param file_path Path to the file on disk.
     * @return Extracted content or a failure result.
     */
    virtual ExtractResult extract(const std::string& file_path) const = 0;
};
