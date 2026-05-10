// Autorzy: Alesia Filinkova, Diana Pelin
// Opis: Ekstraktor PDF. Uruchamia pdftotext jako proces potomny
// (bez powłoki) i odczytuje jego stdout do bufora typu string.

#pragma once

#include "Extractor.h"

/**
 * @brief Strategia ekstrakcji dla plików PDF.
 *
 * Wywołuje pdftotext przez fork+execvp, więc ścieżka pliku trafia
 * bezpośrednio do argv i nigdy nie przechodzi przez powłokę, co
 * eliminuje ryzyko command injection.
 */
class PdfExtractor : public Extractor {
public:
    ExtractResult extract(const std::string& file_path) const override;
};
