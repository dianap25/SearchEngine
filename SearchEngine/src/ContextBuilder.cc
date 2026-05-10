// Authors: Alesia Filinkova, Diana Pelin
// Description: Implementation of ContextBuilder. Performs a
// case-insensitive search by lowering both buffers, then returns a
// substring of the original text clamped to the file boundaries so
// the result preserves the original casing.

#include "ContextBuilder.h"

#include <algorithm>
#include <cctype>

ContextBuilder::ContextBuilder(std::size_t margin_size)
    : margin_size_(margin_size) {
}

std::string ContextBuilder::build(const std::string& text, const std::string& phrase) const {
    if (text.empty() || phrase.empty()) {
        return "";
    }

    auto to_lower = [](std::string value) {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char character) {
            return static_cast<char>(std::tolower(character));
        });
        return value;
    };

    std::string lower_text = to_lower(text);
    std::string lower_phrase = to_lower(phrase);

    std::size_t position = lower_text.find(lower_phrase);

    if (position == std::string::npos) {
        return "";
    }

    std::size_t start = (position > margin_size_) ? position - margin_size_ : 0;
    std::size_t end = position + phrase.size() + margin_size_;

    if (end > text.size()) {
        end = text.size();
    }

    return text.substr(start, end - start);
}
