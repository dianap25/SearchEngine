#pragma once

#include <string>

class SearchEngine {
public:
    void search(const std::string& rootPath, const std::string& phrase);

private:
    std::string buildContext(const std::string& text, const std::string& phrase, std::size_t contextSize = 20);
};