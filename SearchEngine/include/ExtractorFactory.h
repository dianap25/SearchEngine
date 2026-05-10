// Autorzy: Alesia Filinkova, Diana Pelin
// Opis: Fabryka mapująca ścieżkę pliku (po rozszerzeniu) na konkretną
// strategię ekstrakcji, która potrafi go odczytać.

#pragma once

#include "Extractor.h"

#include <memory>
#include <string>

/**
 * @brief Wybiera odpowiednią strategię ekstrakcji dla danej ścieżki
 *        pliku.
 */
class ExtractorFactory {
public:
    /**
     * @brief Buduje ekstraktor na podstawie rozszerzenia ścieżki.
     * @param file_path Ścieżka do pliku na dysku. Plik nie musi
     *                  istnieć; brane pod uwagę jest tylko
     *                  rozszerzenie.
     * @return Wskaźnik własnościowy do konkretnego ekstraktora.
     *         Pliki PDF dostają PdfExtractor; pozostałe (.txt, .tex,
     *         brak rozszerzenia) dostają TextExtractor.
     */
    static std::unique_ptr<Extractor> create(const std::string& file_path);
};
