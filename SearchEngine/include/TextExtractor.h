// Authors: Alesia Filinkova, Diana Pelin
// Description: Plain-text extractor. Reads the file directly from
// disk; suitable for .txt, .tex and files without an extension.

#pragma once

#include "Extractor.h"

/**
 * @brief Extractor strategy for plain text files.
 *
 * Reads the file with std::ifstream and returns its full content.
 * Returns a failure result for missing or empty files so the caller
 * can keep return-code-style error handling.
 */
class TextExtractor : public Extractor {
public:
    ExtractResult extract(const std::string& file_path) const override;
};
