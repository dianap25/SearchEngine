// Autorzy: Alesia Filinkova, Diana Pelin
// Opis: Liczniki zwracane przez IndexService::indexDirectory, dzięki
// którym CLI może raportować ile plików zostało przeskanowanych,
// zaindeksowanych, pominiętych lub uznanych za błędne podczas jednego
// przebiegu indeksacji.

#pragma once

/**
 * @brief Zbiorcze liczniki zebrane podczas pojedynczego przebiegu
 *        indeksacji.
 */
struct IndexSummary {
    int scanned_files = 0;
    int indexed_files = 0;
    int skipped_files = 0;
    int failed_files = 0;
};
