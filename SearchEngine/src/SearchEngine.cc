// Authors: Alesia Filinkova, Diana Pelin
// Description: Legacy implementation of the directory-walking search.
// Currently unreferenced and will be removed in a follow-up commit
// when the CLI no longer needs the SearchEngine class.

#include "SearchEngine.h"

#include "ContextBuilder.h"
#include "ExtractResult.h"
#include "Extractor.h"
#include "FileMetadata.h"
#include "Scanner.h"

#include <iostream>
#include <vector>

void SearchEngine::search(const std::string& root_path, const std::string& phrase) {
    Scanner scanner;
    Extractor extractor;
    ContextBuilder context_builder;

    std::vector<FileMetadata> files = scanner.scan(root_path);

    for (const FileMetadata& file : files) {
        ExtractResult result = extractor.extract(file.path);

        if (!result.success) {
            std::cerr << "[Extractor ERROR] "
                      << file.path << " -> "
                      << result.error_message << "\n";

            continue;
        }

        std::string content = result.content;

        if (content.empty()) {
            continue;
        }

        std::string context = context_builder.build(content, phrase);

        if (!context.empty()) {
            std::cout << "Found in file: " << file.path << "\n";
            std::cout << "Size: " << file.size << " bytes\n";
            std::cout << "Modified time: " << file.modified_time << "\n";
            std::cout << "Context: " << context << "\n\n";
        }
    }
}
