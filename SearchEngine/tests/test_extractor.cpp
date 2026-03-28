#include <gtest/gtest.h>
#include "Extractor.h"
#include <string>


TEST(ExtractorTest, ExtractsTextFromFile) {

    Extractor extractor;
    std::string path = std::string(TEST_SAMPLE_DATA_DIR) + "/example.txt";
    std::string text = extractor.extract(path);

    EXPECT_FALSE(text.empty());
}