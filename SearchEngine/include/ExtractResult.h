// Autorzy: Alesia Filinkova, Diana Pelin
// Opis: Wynik próby ekstrakcji tekstu. Niesie zarówno wydobytą treść,
// jak i komunikat diagnostyczny, dzięki czemu wywołujący mogą dalej
// korzystać z obsługi błędów opartej na kodach powrotu zamiast
// wyjątków.

#pragma once

#include <string>

/**
 * @brief Wynik działania konkretnej implementacji Extractora na
 *        pliku.
 */
struct ExtractResult {
    bool success = false;
    std::string content;
    std::string error_message;

    /**
     * @brief Buduje wynik oznaczający sukces.
     * @param text Wydobyta treść tekstowa.
     */
    static ExtractResult ok(std::string text) {
        return {true, std::move(text), ""};
    }

    /**
     * @brief Buduje wynik oznaczający niepowodzenie.
     * @param error Czytelny dla człowieka opis błędu.
     */
    static ExtractResult fail(std::string error) {
        return {false, "", std::move(error)};
    }
};
