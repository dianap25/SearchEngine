//Alesia Filinkova
//Diana Pelin

#include "SearchEngine.h"
#include "ContextBuilder.h"
#include "Scanner.h"
#include "Extractor.h"
#include "FileMetadata.h"

#include <iostream>
#include <vector>

void SearchEngine::search(const std::string& rootPath, const std::string& phrase) {
    Scanner scanner;
    Extractor extractor;
    ContextBuilder contextBuilder;

    std::vector<FileMetadata> files = scanner.scan(rootPath);

    for (const FileMetadata& file : files) {
        ExtractResult result = extractor.extract(file.path);

        if (!result.success) {
            std::cerr << "[Extractor ERROR] "
                      << file.path << " -> "
                      << result.errorMessage << "\n";

            continue;
        }

        std::string content = result.content;

        if (content.empty()) {
            continue;
        }

        std::string context = contextBuilder.build(content, phrase);

        if (!context.empty()) {
            std::cout << "Found in file: " << file.path << "\n";
            std::cout << "Size: " << file.size << " bytes\n";
            std::cout << "Modified time: " << file.modifiedTime << "\n";
            std::cout << "Context: " << context << "\n\n";
        }
    }
}
