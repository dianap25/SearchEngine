// Authors: Alesia Filinkova, Diana Pelin
// Description: Text extractor that picks an extraction strategy based
// on file extension (.txt/.tex/no-ext = direct read, .pdf = pdftotext).
// Returns an ExtractResult so callers can keep return-code-style
// error handling.

#pragma once

#include "ExtractResult.h"

#include <string>

/**
 * @brief Reads textual content from a file on disk.
 */
class Extractor {
public:
    /**
     * @brief Extract text from the given path.
     * @param file_path Path to the file on disk.
     * @return Extracted content or a failure result.
     */
    ExtractResult extract(const std::string& file_path);

private:
    ExtractResult extractText(const std::string& file_path);
    ExtractResult extractPdf(const std::string& file_path);
    std::string extractPdfInternal(const std::string& file_path);
};
