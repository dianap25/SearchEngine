// Authors: Alesia Filinkova, Diana Pelin
// Description: Helper that crops a short fragment of text around the
// first case-insensitive occurrence of a search phrase. Used by the
// CLI to display a snippet next to each search-content hit.

#pragma once

#include <cstddef>
#include <string>

/**
 * @brief Builds short context windows around a phrase in text.
 */
class ContextBuilder {
public:
    /**
     * @brief Construct with a margin size in characters.
     * @param margin_size Number of characters to include before and
     *                    after the matched phrase. Defaults to 40.
     */
    explicit ContextBuilder(std::size_t margin_size = 40);

    /**
     * @brief Return a fragment of @p text around the first
     *        case-insensitive match of @p phrase.
     * @return The matching fragment or an empty string when @p phrase
     *         is not present.
     */
    std::string build(const std::string& text, const std::string& phrase) const;

private:
    std::size_t margin_size_;
};
