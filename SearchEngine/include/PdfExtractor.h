// Authors: Alesia Filinkova, Diana Pelin
// Description: PDF extractor. Spawns pdftotext as a child process
// (no shell) and reads its stdout into a string.

#pragma once

#include "Extractor.h"

/**
 * @brief Extractor strategy for PDF files.
 *
 * Calls pdftotext through fork+execvp so the file path is passed in
 * argv directly and never goes through a shell, which avoids any
 * command-injection issues.
 */
class PdfExtractor : public Extractor {
public:
    ExtractResult extract(const std::string& file_path) const override;
};
