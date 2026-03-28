#include "Scanner.h"
#include <filesystem>

std::vector<std::string> Scanner::scan(const std::string& path) {
    std::vector<std::string> files;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
        if (entry.is_regular_file()) {
            std::string filePath = entry.path().string();
            if (isSupported(filePath)) {
                files.push_back(filePath);
            }
        }
    }
    return files;
}

bool Scanner::isSupported(const std::string& path) {
    if (path.size() >= 4 && path.substr(path.size() - 4) == ".txt")
        return true;
    if (path.size() >= 4 && path.substr(path.size() - 4) == ".tex")
        return true;
    if (path.size() >= 4 && path.substr(path.size() - 4) == ".pdf")
        return true;
        // files without extension
    return path.find('.') == std::string::npos;
}
