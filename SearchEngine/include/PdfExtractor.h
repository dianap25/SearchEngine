// Authors: Alesia Filinkova, Diana Pelin
// Description: PDF extractor. Spawns pdftotext as a child process
// and reads its stdout into a string buffer.

#pragma once

#include "Extractor.h"

/**
 * @brief Extraction strategy for PDF files.
 *
 * Invokes pdftotext through fork+execvp, so the file path is passed
 * directly to argv and never goes through a shell, which eliminates
 * the risk of command injection.
 */
class PdfExtractor : public Extractor {
public:
    ExtractResult extract(const std::string& file_path) const override;
};
