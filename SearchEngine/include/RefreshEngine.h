// Autorzy: Alesia Filinkova, Diana Pelin
// Opis: Inkrementalny aktualizator. Przechodzi system plików, liczy
// hash każdego pliku i synchronizuje indeks: nowe pliki są
// indeksowane, zmienione re-indeksowane, a pliki, które zniknęły z
// dysku, są usuwane z bazy.

#pragma once

#include "Database.h"

#include <string>

/**
 * @brief Synchronizuje istniejący indeks z aktualnym stanem dysku.
 */
class RefreshEngine {
public:
    /**
     * @brief Synchronizuje drzewo plików w @p root_path z indeksem
     *        w bazie @p db.
     * @param root_path Katalog najwyższego poziomu wcześniej
     *                  zaindeksowany.
     * @param db Baza zawierająca indeks. Schemat jest inicjalizowany
     *           przy pierwszym wywołaniu, dzięki czemu refresh
     *           działa również bez wcześniejszego użycia polecenia
     *           "index".
     */
    void refresh(const std::string& root_path, Database& db);
};
