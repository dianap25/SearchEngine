#include <gtest/gtest.h>
#include "Extractor.h"

#include <string>

#ifndef TEST_SAMPLE_DATA_DIR
#define TEST_SAMPLE_DATA_DIR "../sample_data"
#endif

TEST(ExtractorTest, ExtractsTextFromExistingTxtFile) {
    Extractor extractor;
    std::string path = std::string(TEST_SAMPLE_DATA_DIR) + "/example.txt";

    std::string text = extractor.extract(path);

    EXPECT_FALSE(text.empty());
}

TEST(ExtractorTest, ReturnsEmptyStringForMissingFile) {
    Extractor extractor;

    std::string text = extractor.extract("missing_file.txt");

    EXPECT_TRUE(text.empty());
}