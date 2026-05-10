//Alesia Filinkova
//Diana Pelin

#pragma once

#include <cstddef>
#include <string>

class ContextBuilder {
public:
    explicit ContextBuilder(std::size_t marginSize = 40);

    std::string build(const std::string& text, const std::string& phrase) const;

private:
    std::size_t marginSize_;
};
