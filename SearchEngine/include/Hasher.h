// Autorzy: Alesia Filinkova, Diana Pelin
// Opis: Liczy SHA-256 oparty na bibliotece OpenSSL.
// Używany przez IndexService oraz RefreshEngine do wyliczenia
// fingerprintu treści, dzięki czemu refresh może pominąć
// niezmienione pliki.

#pragma once

#include <string>

/**
 * @brief Bezstanowe opakowanie zwracające szesnastkowy skrót SHA-256
 *        z bufora typu string.
 */
class Hasher {
public:
    /**
     * @brief Liczy szesnastkowy skrót SHA-256 z bufora @p data.
     * @param data Wejściowy bufor (traktowany jako surowe bajty).
     * @return Skrót w postaci 64-znakowego ciągu hex (małe litery).
     */
    static std::string sha256(const std::string& data);
};
