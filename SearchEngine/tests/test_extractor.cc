// Authors: Alesia Filinkova, Diana Pelin
// Description: Unit tests for Extractor. Verify that an existing text
// file is read into ExtractResult successfully and that a missing
// file produces a failure result.

#include <gtest/gtest.h>

#include "ExtractResult.h"
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

TEST(ExtractorTest, ReturnsFailureForMissingFile) {
    Extractor extractor;

    ExtractResult result = extractor.extract("missing_file.txt");

    EXPECT_FALSE(result.success);
    EXPECT_TRUE(result.content.empty());
}
