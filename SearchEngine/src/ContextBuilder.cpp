//Alesia Filinkova
//Diana Pelin

#include "ContextBuilder.h"

#include <algorithm>
#include <cctype>

ContextBuilder::ContextBuilder(std::size_t marginSize)
    : marginSize_(marginSize) {
}

std::string ContextBuilder::build(const std::string& text, const std::string& phrase) const {
    if (text.empty() || phrase.empty()) {
        return "";
    }

    auto toLower = [](std::string value) {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char character) {
            return static_cast<char>(std::tolower(character));
        });
        return value;
    };

    std::string lowerText = toLower(text);
    std::string lowerPhrase = toLower(phrase);

    std::size_t position = lowerText.find(lowerPhrase);

    if (position == std::string::npos) {
        return "";
    }

    std::size_t start = (position > marginSize_) ? position - marginSize_ : 0;
    std::size_t end = position + phrase.size() + marginSize_;

    if (end > text.size()) {
        end = text.size();
    }

    return text.substr(start, end - start);
}
