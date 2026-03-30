#include "SearchEngine.h"
#include "Scanner.h"
#include "Extractor.h"

#include <iostream>
#include <vector>

void SearchEngine::search(const std::string& rootPath, const std::string& phrase) {
    Scanner scanner;
    Extractor extractor;

    std::vector<std::string> files = scanner.scan(rootPath);

    for (const std::string& filePath : files) {
        std::string content = extractor.extract(filePath);

        if (content.empty()) {
            continue;
        }

        if (content.find(phrase) != std::string::npos) {
            std::string context = buildContext(content, phrase);
            std::cout << "Found in file: " << filePath << "\n";
            std::cout << "Context: " << context << "\n\n";
        }
    }
}

std::string SearchEngine::buildContext(const std::string& text, const std::string& phrase, std::size_t contextSize) {
    std::size_t position = text.find(phrase);

    if (position == std::string::npos) {
        return "";
    }

    std::size_t start = (position > contextSize) ? position - contextSize : 0;
    std::size_t end = position + phrase.size() + contextSize;

    if (end > text.size()) {
        end = text.size();
    }

    return text.substr(start, end - start);
}