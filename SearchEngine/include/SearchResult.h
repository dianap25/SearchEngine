// Autorzy: Alesia Filinkova, Diana Pelin
// Opis: Struktura danych opisująca pojedyncze trafienie zwracane
// przez Repository::searchByName / searchByContent i konsumowane
// przez warstwę formatującą CLI.

#pragma once

#include <string>

/**
 * @brief Pojedyncze trafienie zwracane przez wyszukiwanie po nazwie
 *        lub po treści.
 */
struct SearchResult {
    std::string path;
    std::string name;
    int occurrences = 0;
    std::string context;
};
