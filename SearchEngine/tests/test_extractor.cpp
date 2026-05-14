#include <gtest/gtest.h>
#include "Extractor.h"

#include <string>

#ifndef TEST_SAMPLE_DATA_DIR
#define TEST_SAMPLE_DATA_DIR "../sample_data"
#endif

TEST(ExtractorTest, ExtractsTextFromExistingTxtFile) {
    Extractor extractor;
    std::string path = std::string(TEST_SAMPLE_DATA_DIR) + "/example.txt";

    ExtractResult result = extractor.extract(path);

    EXPECT_TRUE(result.success);
    EXPECT_FALSE(result.content.empty());
}

TEST(ExtractorTest, ReturnsErrorForMissingFile) {
    Extractor extractor;

    ExtractResult result = extractor.extract("missing_file.txt");

    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.errorMessage.empty());
}