// Autorzy: Alesia Filinkova, Diana Pelin
// Opis: Rekurencyjny skaner katalogów. Przechodzi drzewo katalogów
// i buduje rekordy FileMetadata dla plików o obsługiwanych
// rozszerzeniach (.txt, .tex, .pdf, brak rozszerzenia).

#pragma once

#include "FileMetadata.h"

#include <filesystem>
#include <string>
#include <vector>

/**
 * @brief Przechodzi drzewo katalogów i emituje FileMetadata dla
 *        plików nadających się do indeksowania.
 */
class Scanner {
public:
    /**
     * @brief Rekurencyjnie skanuje katalog.
     * @param root_path Katalog najwyższego poziomu.
     * @return Metadane każdego obsługiwanego, zwykłego pliku.
     */
    std::vector<FileMetadata> scan(const std::string& root_path);

private:
    bool isSupported(const std::filesystem::path& path) const;
    FileMetadata buildMetadata(const std::filesystem::directory_entry& entry) const;
    std::int64_t toUnixTimestamp(const std::filesystem::file_time_type& file_time) const;
};
