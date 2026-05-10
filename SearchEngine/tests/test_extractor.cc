// Authors: Alesia Filinkova, Diana Pelin
// Description: Unit tests for the Extractor strategy hierarchy. Cover
// TextExtractor end-to-end against the sample_data fixture, and
// PdfExtractor against the same path; the PDF case is skipped when
// pdftotext is not on PATH so the suite stays portable.

#include <gtest/gtest.h>

#include "ExtractResult.h"
#include "ExtractorFactory.h"
#include "PdfExtractor.h"
#include "TextExtractor.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>

#ifndef TEST_SAMPLE_DATA_DIR
#define TEST_SAMPLE_DATA_DIR "../sample_data"
#endif

namespace {

bool pdftotextAvailable() {
    return std::system("command -v pdftotext > /dev/null 2>&1") == 0;
}

} // namespace

TEST(TextExtractorTest, ReadsExistingTxtFile) {
    TextExtractor extractor;
    std::string path = std::string(TEST_SAMPLE_DATA_DIR) + "/example.txt";

    ExtractResult result = extractor.extract(path);

    EXPECT_TRUE(result.success);
    EXPECT_FALSE(result.content.empty());
}

TEST(TextExtractorTest, FailsForMissingFile) {
    TextExtractor extractor;

    ExtractResult result = extractor.extract("missing_file.txt");

    EXPECT_FALSE(result.success);
    EXPECT_TRUE(result.content.empty());
}

TEST(TextExtractorTest, FailsForEmptyFile) {
    const std::filesystem::path empty_path = "test_extractor_empty.txt";
    {
        std::ofstream file(empty_path);
    }

    TextExtractor extractor;
    ExtractResult result = extractor.extract(empty_path.string());

    EXPECT_FALSE(result.success);

    std::filesystem::remove(empty_path);
}

TEST(PdfExtractorTest, FailsForMissingPdf) {
    PdfExtractor extractor;

    ExtractResult result = extractor.extract("missing_file.pdf");

    EXPECT_FALSE(result.success);
}

TEST(PdfExtractorTest, ExtractsContentFromRealPdfWhenAvailable) {
    if (!pdftotextAvailable()) {
        GTEST_SKIP() << "pdftotext not on PATH";
    }

    PdfExtractor extractor;
    const std::filesystem::path pdf_path = std::string(TEST_SAMPLE_DATA_DIR) + "/example.pdf";

    if (!std::filesystem::exists(pdf_path)) {
        GTEST_SKIP() << "no sample pdf at " << pdf_path;
    }

    ExtractResult result = extractor.extract(pdf_path.string());

    EXPECT_TRUE(result.success);
    EXPECT_FALSE(result.content.empty());
}

TEST(ExtractorFactoryTest, ReturnsTextExtractorForTxt) {
    auto extractor = ExtractorFactory::create("foo.txt");
    EXPECT_NE(dynamic_cast<TextExtractor*>(extractor.get()), nullptr);
}

TEST(ExtractorFactoryTest, ReturnsTextExtractorForTex) {
    auto extractor = ExtractorFactory::create("foo.tex");
    EXPECT_NE(dynamic_cast<TextExtractor*>(extractor.get()), nullptr);
}

TEST(ExtractorFactoryTest, ReturnsPdfExtractorForPdf) {
    auto extractor = ExtractorFactory::create("foo.pdf");
    EXPECT_NE(dynamic_cast<PdfExtractor*>(extractor.get()), nullptr);
}

TEST(ExtractorFactoryTest, ReturnsTextExtractorForUnknownExtension) {
    auto extractor = ExtractorFactory::create("foo");
    EXPECT_NE(dynamic_cast<TextExtractor*>(extractor.get()), nullptr);
}
