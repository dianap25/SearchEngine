// Authors: Alesia Filinkova, Diana Pelin
// Description: Helper that cuts out a short window of text around the
// first occurrence of the search phrase. Used by the CLI to print a
// context snippet next to every search-content hit.

#pragma once

#include <cstddef>
#include <string>

/**
 * @brief Builds short context windows around a phrase in a text.
 */
class ContextBuilder {
public:
    /**
     * @brief Constructs the helper with a margin size in characters.
     * @param margin_size Number of characters before and after the
     *                    match to include in the result. Defaults to
     *                    40.
     */
    explicit ContextBuilder(std::size_t margin_size = 40);

    /**
     * @brief Returns the fragment of @p text around the first
     *        case-insensitive occurrence of @p phrase.
     * @return The matching fragment, or an empty string when @p phrase
     *         is not present in the text.
     */
    std::string build(const std::string& text, const std::string& phrase) const;

private:
    std::size_t margin_size_;
};
