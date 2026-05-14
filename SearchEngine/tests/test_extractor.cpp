#include <gtest/gtest.h>
#include "Extractor.h"
#include "TextExtractor.h"
#include "PdfExtractor.h"
#include "ExtractorFactory.h"

#include <string>
#include <fstream>
#include <filesystem>

#ifndef TEST_SAMPLE_DATA_DIR
#define TEST_SAMPLE_DATA_DIR "../sample_data"
#endif

namespace fs = std::filesystem;

class TextExtractorTest : public ::testing::Test {
protected:
    void SetUp() override {
        testDir_ = std::string(TEST_SAMPLE_DATA_DIR) + "/text_test";
        fs::create_directories(testDir_);
    }

    void TearDown() override {
        fs::remove_all(testDir_);
    }

    std::string createTestFile(const std::string& name, const std::string& content) {
        std::string path = testDir_ + "/" + name;
        std::ofstream file(path);
        file << content;
        file.close();
        return path;
    }

    std::string testDir_;
};

TEST_F(TextExtractorTest, ExtractsTextFromExistingTxtFile) {
    TextExtractor extractor;
    std::string path = createTestFile("example.txt", "Hello, World!");

    ExtractResult result = extractor.extract(path);

    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.content, "Hello, World!");
}

TEST_F(TextExtractorTest, ExtractsTextFromFileWithoutExtension) {
    TextExtractor extractor;
    std::string path = createTestFile("no_extension", "Content without extension");

    ExtractResult result = extractor.extract(path);

    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.content, "Content without extension");
}

TEST_F(TextExtractorTest, ExtractsTextFromTexFile) {
    TextExtractor extractor;
    std::string path = createTestFile("document.tex", "\\documentclass{article}\nHello");

    ExtractResult result = extractor.extract(path);

    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.content, "\\documentclass{article}\nHello");
}

TEST_F(TextExtractorTest, ReturnsErrorForMissingFile) {
    TextExtractor extractor;

    ExtractResult result = extractor.extract("missing_file.txt");

    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.error_message.empty());
    EXPECT_EQ(result.error_message, "Plik nie istnieje");
}

TEST_F(TextExtractorTest, ReturnsErrorForEmptyFile) {
    TextExtractor extractor;
    std::string path = createTestFile("empty.txt", "");

    ExtractResult result = extractor.extract(path);

    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.error_message, "Pusty plik tekstowy");
}

TEST_F(TextExtractorTest, ReturnsErrorForDirectory) {
    TextExtractor extractor;

    ExtractResult result = extractor.extract(testDir_);

    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.error_message, "Plik nie istnieje");
}

TEST_F(TextExtractorTest, ExtractsMultilineText) {
    TextExtractor extractor;
    std::string content = "Line 1\nLine 2\nLine 3";
    std::string path = createTestFile("multiline.txt", content);

    ExtractResult result = extractor.extract(path);

    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.content, content);
}

TEST_F(TextExtractorTest, ExtractsLargeText) {
    TextExtractor extractor;
    std::string largeContent(10000, 'A');
    std::string path = createTestFile("large.txt", largeContent);

    ExtractResult result = extractor.extract(path);

    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.content.size(), 10000);
    EXPECT_EQ(result.content, largeContent);
}


class PdfExtractorTest : public ::testing::Test {
protected:
    void SetUp() override {
        testDir_ = std::string(TEST_SAMPLE_DATA_DIR) + "/pdf_test";
        fs::create_directories(testDir_);
        
        pdftotextAvailable_ = (system("which pdftotext > /dev/null 2>&1") == 0);
    }

    void TearDown() override {
        fs::remove_all(testDir_);
    }

    std::string createTestFile(const std::string& name, const std::string& content) {
        std::string path = testDir_ + "/" + name;
        std::ofstream file(path);
        file << content;
        file.close();
        return path;
    }

    std::string createPdfPath(const std::string& name) {
        return std::string(TEST_SAMPLE_DATA_DIR) + "/" + name;
    }

    std::string testDir_;
    bool pdftotextAvailable_;
};

TEST_F(PdfExtractorTest, ExtractsTextFromExistingPdfFile) {
    if (!pdftotextAvailable_) {
        GTEST_SKIP() << "pdftotext not installed";
    }
    
    PdfExtractor extractor;
    std::string path = createPdfPath("sample.pdf");

    ExtractResult result = extractor.extract(path);

    if (fs::exists(path)) {
        EXPECT_TRUE(result.success);
        EXPECT_FALSE(result.content.empty());
    } else {
        GTEST_SKIP() << "sample.pdf not found in " << TEST_SAMPLE_DATA_DIR;
    }
}

TEST_F(PdfExtractorTest, ReturnsErrorForMissingPdfFile) {
    PdfExtractor extractor;

    ExtractResult result = extractor.extract("missing.pdf");

    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.error_message, "Plik nie istnieje");
}

TEST_F(PdfExtractorTest, ReturnsErrorForNonPdfFile) {
    if (!pdftotextAvailable_) {
        GTEST_SKIP() << "pdftotext not installed";
    }
    
    PdfExtractor extractor;
    std::string path = createTestFile("not_pdf.txt", "This is not a PDF");

    ExtractResult result = extractor.extract(path);

    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.error_message.empty());
}

class ExtractorFactoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        testDir_ = std::string(TEST_SAMPLE_DATA_DIR) + "/factory_test";
        fs::create_directories(testDir_);
    }

    void TearDown() override {
        fs::remove_all(testDir_);
    }

    std::string createTestFile(const std::string& name) {
        std::string path = testDir_ + "/" + name;
        std::ofstream file(path);
        file << "test content";
        file.close();
        return path;
    }

    std::string testDir_;
};

TEST_F(ExtractorFactoryTest, CreatesTextExtractorForTxtFile) {
    std::string path = createTestFile("document.txt");
    auto extractor = ExtractorFactory::create(path);

    EXPECT_NE(extractor, nullptr);
    EXPECT_NE(dynamic_cast<TextExtractor*>(extractor.get()), nullptr);
}

TEST_F(ExtractorFactoryTest, CreatesTextExtractorForTexFile) {
    std::string path = createTestFile("document.tex");
    auto extractor = ExtractorFactory::create(path);

    EXPECT_NE(extractor, nullptr);
    EXPECT_NE(dynamic_cast<TextExtractor*>(extractor.get()), nullptr);
}

TEST_F(ExtractorFactoryTest, CreatesTextExtractorForFileWithoutExtension) {
    std::string path = createTestFile("no_extension");
    auto extractor = ExtractorFactory::create(path);

    EXPECT_NE(extractor, nullptr);
    EXPECT_NE(dynamic_cast<TextExtractor*>(extractor.get()), nullptr);
}

TEST_F(ExtractorFactoryTest, CreatesPdfExtractorForPdfFile) {
    std::string path = createTestFile("document.pdf");
    auto extractor = ExtractorFactory::create(path);

    EXPECT_NE(extractor, nullptr);
    EXPECT_NE(dynamic_cast<PdfExtractor*>(extractor.get()), nullptr);
}

TEST_F(ExtractorFactoryTest, CreatesPdfExtractorForUpperCasePdf) {
    std::string path = createTestFile("document.PDF");
    auto extractor = ExtractorFactory::create(path);

    EXPECT_NE(extractor, nullptr);
}

TEST_F(ExtractorFactoryTest, ReturnsUniquePtrForEachCall) {
    std::string txtPath = createTestFile("doc1.txt");
    std::string pdfPath = createTestFile("doc2.pdf");
    
    auto extractor1 = ExtractorFactory::create(txtPath);
    auto extractor2 = ExtractorFactory::create(txtPath);

    EXPECT_NE(extractor1.get(), extractor2.get());
}


TEST(ExtractResultTest, OkCreatesSuccessResult) {
    ExtractResult result = ExtractResult::ok("Hello");

    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.content, "Hello");
    EXPECT_TRUE(result.error_message.empty());
}

TEST(ExtractResultTest, FailCreatesFailureResult) {
    ExtractResult result = ExtractResult::fail("Error message");

    EXPECT_FALSE(result.success);
    EXPECT_TRUE(result.content.empty());
    EXPECT_EQ(result.error_message, "Error message");
}

TEST(ExtractResultTest, MoveConstructorWorks) {
    ExtractResult original = ExtractResult::ok("Content");
    ExtractResult moved = std::move(original);

    EXPECT_TRUE(moved.success);
    EXPECT_EQ(moved.content, "Content");
}

TEST(ExtractResultTest, MoveAssignmentWorks) {
    ExtractResult original = ExtractResult::ok("Content");
    ExtractResult target = ExtractResult::fail("Old");
    target = std::move(original);

    EXPECT_TRUE(target.success);
    EXPECT_EQ(target.content, "Content");
}

class IntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        testDir_ = std::string(TEST_SAMPLE_DATA_DIR) + "/integration";
        fs::create_directories(testDir_);
    }

    void TearDown() override {
        fs::remove_all(testDir_);
    }

    std::string createTextFile(const std::string& name, const std::string& content) {
        std::string path = testDir_ + "/" + name;
        std::ofstream file(path);
        file << content;
        file.close();
        return path;
    }

    std::string testDir_;
};

TEST_F(IntegrationTest, FactoryAndTextExtractorWorkTogether) {
    std::string path = createTextFile("test.txt", "Integration test content");
    
    auto extractor = ExtractorFactory::create(path);
    ExtractResult result = extractor->extract(path);

    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.content, "Integration test content");
}

TEST_F(IntegrationTest, FactoryReturnsProperExtractorTypeBasedOnExtension) {
    std::string txtPath = createTextFile("a.txt", "");
    std::string texPath = createTextFile("b.tex", "");
    std::string pdfPath = createTextFile("c.pdf", "");
    std::string noExtPath = createTextFile("no_ext", "");

    auto txtExtractor = ExtractorFactory::create(txtPath);
    auto texExtractor = ExtractorFactory::create(texPath);
    auto pdfExtractor = ExtractorFactory::create(pdfPath);
    auto noExtExtractor = ExtractorFactory::create(noExtPath);

    EXPECT_NE(dynamic_cast<TextExtractor*>(txtExtractor.get()), nullptr);
    EXPECT_NE(dynamic_cast<TextExtractor*>(texExtractor.get()), nullptr);
    EXPECT_NE(dynamic_cast<PdfExtractor*>(pdfExtractor.get()), nullptr);
    EXPECT_NE(dynamic_cast<TextExtractor*>(noExtExtractor.get()), nullptr);
}
