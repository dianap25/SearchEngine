// Autorzy: Alesia Filinkova, Diana Pelin
// Opis: Testy jednostkowe klasy Scanner. Pokrywają poprawne skany
// fixtury sample_data, wypełnianie metadanych, filtrowanie po
// obsługiwanych rozszerzeniach oraz spokojne radzenie sobie z
// nieistniejącymi katalogami.

#include <gtest/gtest.h>

#include "Scanner.h"

#include <algorithm>
#include <string>
#include <vector>

#ifndef TEST_SAMPLE_DATA_DIR
#define TEST_SAMPLE_DATA_DIR "../sample_data"
#endif

TEST(ScannerTest, FindsSupportedFiles) {
    Scanner scanner;

    std::vector<FileMetadata> files = scanner.scan(TEST_SAMPLE_DATA_DIR);

    EXPECT_FALSE(files.empty());
}

TEST(ScannerTest, ReturnsFileMetadata) {
    Scanner scanner;

    std::vector<FileMetadata> files = scanner.scan(TEST_SAMPLE_DATA_DIR);

    ASSERT_FALSE(files.empty());

    const FileMetadata& file = files.front();

    EXPECT_FALSE(file.path.empty());
    EXPECT_FALSE(file.name.empty());
    EXPECT_GE(file.size, 0);
    EXPECT_GT(file.modified_time, 0);
}

TEST(ScannerTest, SupportsTxtAndTexFiles) {
    Scanner scanner;

    std::vector<FileMetadata> files = scanner.scan(TEST_SAMPLE_DATA_DIR);

    const bool has_txt = std::any_of(files.begin(), files.end(), [](const FileMetadata& file) {
        return file.extension == ".txt";
    });

    const bool has_tex = std::any_of(files.begin(), files.end(), [](const FileMetadata& file) {
        return file.extension == ".tex";
    });

    EXPECT_TRUE(has_txt);
    EXPECT_TRUE(has_tex);
}

TEST(ScannerTest, ReturnsEmptyVectorForMissingDirectory) {
    Scanner scanner;

    std::vector<FileMetadata> files = scanner.scan("missing_directory");

    EXPECT_TRUE(files.empty());
}
