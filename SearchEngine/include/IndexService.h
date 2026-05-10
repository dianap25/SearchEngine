// Autorzy: Alesia Filinkova, Diana Pelin
// Opis: Spina ze sobą Scanner, Extractor, Repository i Indexer pod jednym
// punktem wejścia indexDirectory(), używanym przez polecenie CLI
// "index".

#pragma once

#include "Database.h"
#include "IndexSummary.h"

#include <string>

/**
 * @brief Indeksuje każdy obsługiwany plik w katalogu w trybie
 *        "transakcja na plik".
 */
class IndexService {
public:
    explicit IndexService(Database& database);

    /**
     * @brief Przechodzi @p root_path, ekstrahuje treść i indeksuje
     *        wszystko.
     * @param root_path Katalog do przeskanowania rekurencyjnie.
     * @return Liczniki opisujące przebieg (przeskanowane,
     *         zaindeksowane, pominięte, błędne).
     */
    IndexSummary indexDirectory(const std::string& root_path);

private:
    Database& database_;
};
