// Autorzy: Alesia Filinkova, Diana Pelin
// Opis: Struktura danych opisująca pojedynczy plik na dysku oraz
// stan, jaki o tym pliku przechowuje indeks (id w bazie, hash
// zawartości). Używana jako wspólny typ wymiany między klasami
// Scanner, Repository, Indexer i RefreshEngine.

#pragma once

#include <cstdint>
#include <string>

/**
 * @brief Migawka pliku w postaci, w jakiej istnieje na dysku oraz w
 *        indeksie.
 *
 * Scanner wypełnia pola pochodzące z dysku (path, name, extension,
 * size, modified_time). Repository wypełnia @c id i @c content_hash
 * z bazy danych. RefreshEngine porównuje hashe, aby zdecydować, czy
 * plik wymaga ponownej indeksacji.
 */
struct FileMetadata {
    int id = 0;
    std::string path;
    std::string name;
    std::string extension;
    std::uintmax_t size = 0;
    std::int64_t modified_time = 0;
    std::string content_hash;
};
