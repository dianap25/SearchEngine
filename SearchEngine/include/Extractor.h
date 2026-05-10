// Autorzy: Alesia Filinkova, Diana Pelin
// Opis: Abstrakcyjna baza hierarchii ekstraktorów (wzorzec strategii).
// Konkretne podklasy (TextExtractor, PdfExtractor) wpinają się do
// jednego dyspozytora (ExtractorFactory), który dobiera odpowiednią
// strategię na podstawie rozszerzenia pliku.

#pragma once

#include "ExtractResult.h"

#include <string>

/**
 * @brief Interfejs strategii czytającej zawartość tekstową z pliku.
 */
class Extractor {
public:
    virtual ~Extractor() = default;

    /**
     * @brief Ekstrahuje treść tekstową z pliku @p file_path.
     * @param file_path Ścieżka do pliku na dysku.
     * @return Wydobyta treść albo wynik niepowodzenia.
     */
    virtual ExtractResult extract(const std::string& file_path) const = 0;
};
