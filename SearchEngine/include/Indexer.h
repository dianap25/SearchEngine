// Autorzy: Alesia Filinkova, Diana Pelin
// Opis: Dzieli wydobyty tekst na
// znormalizowane termy wraz z ich pozycjami i zapisuje je przez
// Repository, tak aby search-content mogło je później odnaleźć.

#pragma once

#include "Repository.h"

#include <string>
#include <utility>
#include <vector>

/**
 * @brief Buduje indeks odwrócony dla pojedynczego pliku.
 *
 * Trzyma referencję do Repository, przez które zapisuje dane;
 * obiekt Repository musi przeżyć Indexer.
 */
class Indexer {
public:
    explicit Indexer(Repository& repository);

    /**
     * @brief Tokenizuje @p content i zapisuje wszystkie postingi
     *        dla pliku.
     * @param file_id Id pliku w bazie.
     * @param content Wydobyty tekst pliku.
     * @return true jeśli wszystkie postingi zostały zapisane
     *         poprawnie.
     */
    bool indexFile(int file_id, const std::string& content);

    /**
     * @brief Dzieli tekst na pary (znormalizowany term, pozycja).
     * @param content Surowy wydobyty tekst.
     */
    std::vector<std::pair<std::string, int>> tokenize(const std::string& content) const;

private:
    std::string normalizeToken(const std::string& token) const;

    Repository& repository_;
};
