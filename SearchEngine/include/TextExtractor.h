// Autorzy: Alesia Filinkova, Diana Pelin
// Opis: Ekstraktor zwykłego tekstu. Czyta plik bezpośrednio z dysku;
// odpowiedni dla rozszerzeń .txt, .tex oraz plików bez rozszerzenia.

#pragma once

#include "Extractor.h"

/**
 * @brief Strategia ekstrakcji dla plików tekstowych.
 *
 * Czyta plik za pomocą std::ifstream i zwraca jego pełną treść.
 * Zwraca wynik niepowodzenia dla plików nieistniejących lub pustych,
 * tak aby wywołujący mogli pozostać przy obsłudze błędów opartej na
 * kodach powrotu.
 */
class TextExtractor : public Extractor {
public:
    ExtractResult extract(const std::string& file_path) const override;
};
