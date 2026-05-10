// Autorzy: Alesia Filinkova, Diana Pelin
// Opis: Implementacja klasy Scanner. Przechodzi drzewo katalogów
// przez std::filesystem, filtruje pliki po obsługiwanych
// rozszerzeniach i konwertuje zależny od platformy file_time_type na
// uniksowy timestamp, który reszta silnika może zapisać i porównywać.

#include "Scanner.h"

#include <chrono>
#include <iostream>

std::vector<FileMetadata> Scanner::scan(const std::string& root_path) {
    std::vector<FileMetadata> files;

    std::filesystem::path root(root_path);

    if (!std::filesystem::exists(root)) {
        std::cerr << "Scan path does not exist: " << root_path << "\n";
        return files;
    }

    if (!std::filesystem::is_directory(root)) {
        std::cerr << "Scan path is not a directory: " << root_path << "\n";
        return files;
    }

    std::filesystem::recursive_directory_iterator iterator(
        root,
        std::filesystem::directory_options::skip_permission_denied
    );

    for (const auto& entry : iterator) {
        try {
            if (!entry.is_regular_file()) {
                continue;
            }

            if (!isSupported(entry.path())) {
                continue;
            }

            files.push_back(buildMetadata(entry));
        } catch (const std::filesystem::filesystem_error& error) {
            std::cerr << "Failed to read file metadata: "
                      << entry.path().string()
                      << " | "
                      << error.what()
                      << "\n";
        }
    }

    return files;
}

bool Scanner::isSupported(const std::filesystem::path& path) const {
    std::string extension = path.extension().string();

    return extension == ".txt"
        || extension == ".tex"
        || extension == ".pdf"
        || extension.empty();
}

FileMetadata Scanner::buildMetadata(const std::filesystem::directory_entry& entry) const {
    const std::filesystem::path& path = entry.path();

    FileMetadata metadata;
    metadata.path = path.string();
    metadata.name = path.filename().string();
    metadata.extension = path.extension().string();
    metadata.size = entry.file_size();
    metadata.modified_time = toUnixTimestamp(entry.last_write_time());

    return metadata;
}

std::int64_t Scanner::toUnixTimestamp(const std::filesystem::file_time_type& file_time) const {
    const auto system_time = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        file_time - std::filesystem::file_time_type::clock::now()
        + std::chrono::system_clock::now()
    );

    return std::chrono::duration_cast<std::chrono::seconds>(
        system_time.time_since_epoch()
    ).count();
}
