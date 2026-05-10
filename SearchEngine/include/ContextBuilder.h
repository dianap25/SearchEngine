// Autorzy: Alesia Filinkova, Diana Pelin
// Opis: Pomocnik wycinający krótki fragment tekstu wokół pierwszego
// wystąpienia szukanej frazy (porównywanie niezależne od wielkości
// liter). Używany przez CLI do wyświetlenia kontekstu obok każdego
// trafienia search-content.

#pragma once

#include <cstddef>
#include <string>

/**
 * @brief Buduje krótkie okna kontekstu wokół frazy w tekście.
 */
class ContextBuilder {
public:
    /**
     * @brief Konstruuje obiekt z rozmiarem marginesu w znakach.
     * @param margin_size Liczba znaków przed i po dopasowaniu, które
     *                    mają znaleźć się w wyniku. Domyślnie 40.
     */
    explicit ContextBuilder(std::size_t margin_size = 40);

    /**
     * @brief Zwraca fragment @p text wokół pierwszego, niewrażliwego
     *        na wielkość liter, wystąpienia @p phrase.
     * @return Pasujący fragment albo pusty string, gdy @p phrase nie
     *         występuje w tekście.
     */
    std::string build(const std::string& text, const std::string& phrase) const;

private:
    std::size_t margin_size_;
};
