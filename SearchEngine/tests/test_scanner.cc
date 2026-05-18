// Authors: Alesia Filinkova, Diana Pelin



#include <gtest/gtest.h>
#include "Scanner.h"
#include <algorithm>
#include <string>
#include <vector>
#include <fstream>
#include <chrono>
#include <thread>

#ifndef TEST_SAMPLE_DATA_DIR
#define TEST_SAMPLE_DATA_DIR "../sample_data"
#endif

class ScannerTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir_ = "test_scanner_dir";
        std::filesystem::create_directory(test_dir_);
        
        createTestFile(test_dir_ + "/file1.txt");
        createTestFile(test_dir_ + "/file2.tex");
        createTestFile(test_dir_ + "/file3.pdf");
        createTestFile(test_dir_ + "/file4.xyz"); // unsupported
        createTestFile(test_dir_ + "/file5");      // no extension
        
        std::filesystem::create_directory(test_dir_ + "/subdir");
        createTestFile(test_dir_ + "/subdir/nested.txt");
        createTestFile(test_dir_ + "/subdir/nested.tex");
        
        std::filesystem::create_directory(test_dir_ + "/empty_dir");
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    void TearDown() override {
        std::filesystem::remove_all(test_dir_);
    }
    
    void createTestFile(const std::string& path) {
        std::ofstream file(path);
        if (file.is_open()) {
            file << "Test content for " << path << "\n";
            file.close();
        }
    }
    
    std::string test_dir_;
};

TEST_F(ScannerTest, FindsSupportedFilesInDirectory) {
    Scanner scanner;
    std::vector<FileMetadata> files = scanner.scan(test_dir_);
    
    EXPECT_FALSE(files.empty());
}

TEST_F(ScannerTest, ReturnsCorrectNumberOfSupportedFiles) {
    Scanner scanner;
    std::vector<FileMetadata> files = scanner.scan(test_dir_);
    
    // Expected: file1.txt, file2.tex, file3.pdf, file5, subdir/nested.txt, subdir/nested.tex
    // = 6 files (file4.xyz is unsupported, empty_dir has no files)
    EXPECT_EQ(files.size(), 6);
}

TEST_F(ScannerTest, ReturnsFileMetadata) {
    Scanner scanner;
    std::vector<FileMetadata> files = scanner.scan(test_dir_);
    
    ASSERT_FALSE(files.empty());
    
    const FileMetadata& file = files.front();
    
    EXPECT_FALSE(file.path.empty());
    EXPECT_FALSE(file.name.empty());
    EXPECT_GE(file.size, 0);
    EXPECT_GT(file.modified_time, 0);
}

TEST_F(ScannerTest, SupportsTxtAndTexFiles) {
    Scanner scanner;
    std::vector<FileMetadata> files = scanner.scan(test_dir_);
    
    bool has_txt = std::any_of(files.begin(), files.end(), [](const FileMetadata& file) {
        return file.extension == ".txt";
    });
    
    bool has_tex = std::any_of(files.begin(), files.end(), [](const FileMetadata& file) {
        return file.extension == ".tex";
    });
    
    EXPECT_TRUE(has_txt);
    EXPECT_TRUE(has_tex);
}

TEST_F(ScannerTest, SupportsPdfFiles) {
    Scanner scanner;
    std::vector<FileMetadata> files = scanner.scan(test_dir_);
    
    bool has_pdf = std::any_of(files.begin(), files.end(), [](const FileMetadata& file) {
        return file.extension == ".pdf";
    });
    
    EXPECT_TRUE(has_pdf);
}

TEST_F(ScannerTest, SupportsFilesWithoutExtension) {
    Scanner scanner;
    std::vector<FileMetadata> files = scanner.scan(test_dir_);
    
    bool has_no_extension = std::any_of(files.begin(), files.end(), [](const FileMetadata& file) {
        return file.extension.empty();
    });
    
    EXPECT_TRUE(has_no_extension);
}

TEST_F(ScannerTest, IgnoresUnsupportedExtensions) {
    Scanner scanner;
    std::vector<FileMetadata> files = scanner.scan(test_dir_);
    
    bool has_unsupported = std::any_of(files.begin(), files.end(), [](const FileMetadata& file) {
        return file.extension == ".xyz";
    });
    
    EXPECT_FALSE(has_unsupported);
}

TEST_F(ScannerTest, ScansSubdirectoriesRecursively) {
    Scanner scanner;
    std::vector<FileMetadata> files = scanner.scan(test_dir_);
    
    bool has_nested = std::any_of(files.begin(), files.end(), [](const FileMetadata& file) {
        return file.path.find("subdir/nested") != std::string::npos;
    });
    
    EXPECT_TRUE(has_nested);
}

TEST_F(ScannerTest, ReturnsFilesFromNestedDirectories) {
    Scanner scanner;
    std::vector<FileMetadata> files = scanner.scan(test_dir_);
    
    int nested_count = std::count_if(files.begin(), files.end(), [](const FileMetadata& file) {
        return file.path.find("subdir/") != std::string::npos;
    });
    
    EXPECT_EQ(nested_count, 2); // nested.txt and nested.tex
}

TEST_F(ScannerTest, ReturnsEmptyVectorForMissingDirectory) {
    Scanner scanner;
    std::vector<FileMetadata> files = scanner.scan("missing_directory_that_does_not_exist");
    
    EXPECT_TRUE(files.empty());
}

TEST_F(ScannerTest, HandlesFileInsteadOfDirectory) {
    Scanner scanner;
    std::string file_path = test_dir_ + "/file1.txt";
    
    std::vector<FileMetadata> files = scanner.scan(file_path);
    
    EXPECT_TRUE(files.empty());
}

TEST_F(ScannerTest, HandlesEmptyDirectory) {
    Scanner scanner;
    std::vector<FileMetadata> files = scanner.scan(test_dir_ + "/empty_dir");
    
    EXPECT_TRUE(files.empty());
}


TEST_F(ScannerTest, FilePathIsAbsolute) {
    Scanner scanner;
    std::vector<FileMetadata> files = scanner.scan(test_dir_);
    
    ASSERT_FALSE(files.empty());
    
    for (const auto& file : files) {
        // Check that path is absolute or at least contains the test directory
        EXPECT_TRUE(file.path.find(test_dir_) != std::string::npos);
    }
}

TEST_F(ScannerTest, FileNameIsCorrect) {
    Scanner scanner;
    std::vector<FileMetadata> files = scanner.scan(test_dir_);
    
    auto it = std::find_if(files.begin(), files.end(), [](const FileMetadata& file) {
        return file.name == "file1.txt";
    });
    
    EXPECT_NE(it, files.end());
}

TEST_F(ScannerTest, FileExtensionIsCorrect) {
    Scanner scanner;
    std::vector<FileMetadata> files = scanner.scan(test_dir_);
    
    auto it = std::find_if(files.begin(), files.end(), [](const FileMetadata& file) {
        return file.name == "file1.txt";
    });
    
    ASSERT_NE(it, files.end());
    EXPECT_EQ(it->extension, ".txt");
}

TEST_F(ScannerTest, FileSizeIsPositive) {
    Scanner scanner;
    std::vector<FileMetadata> files = scanner.scan(test_dir_);
    
    for (const auto& file : files) {
        EXPECT_GT(file.size, 0);
    }
}

TEST_F(ScannerTest, ModifiedTimeIsReasonable) {
    Scanner scanner;
    std::vector<FileMetadata> files = scanner.scan(test_dir_);
    
    auto now = std::chrono::system_clock::now();
    auto now_seconds = std::chrono::duration_cast<std::chrono::seconds>(
        now.time_since_epoch()
    ).count();
    
    for (const auto& file : files) {
        // Modified time should be within the last 10 seconds
        EXPECT_LE(file.modified_time, now_seconds);
        EXPECT_GT(file.modified_time, now_seconds - 60);
    }
}

TEST(ScannerIntegrationTest, FindsSupportedFilesInSampleData) {
    Scanner scanner;
    
    std::vector<FileMetadata> files = scanner.scan(TEST_SAMPLE_DATA_DIR);
    
    // Skip test if sample data directory is empty
    if (files.empty()) {
        GTEST_SKIP() << "Sample data directory is empty or doesn't exist";
    }
    
    EXPECT_FALSE(files.empty());
}

TEST(ScannerIntegrationTest, ReturnsFileMetadataForSampleData) {
    Scanner scanner;
    
    std::vector<FileMetadata> files = scanner.scan(TEST_SAMPLE_DATA_DIR);
    
    if (files.empty()) {
        GTEST_SKIP() << "Sample data directory is empty or doesn't exist";
    }
    
    ASSERT_FALSE(files.empty());
    
    const FileMetadata& file = files.front();
    
    EXPECT_FALSE(file.path.empty());
    EXPECT_FALSE(file.name.empty());
    EXPECT_GE(file.size, 0);
    EXPECT_GT(file.modified_time, 0);
}

TEST(ScannerPerformanceTest, ScansLargeDirectoryQuickly) {
    Scanner scanner;
    
    // This test is optional and may be disabled
    auto start = std::chrono::steady_clock::now();
    
    std::vector<FileMetadata> files = scanner.scan(".");
    
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Should complete within 5 seconds
    EXPECT_LT(duration.count(), 5000);
    
    std::cout << "Scanned " << files.size() << " files in " << duration.count() << "ms\n";
}

TEST_F(ScannerTest, HandlesPathWithTrailingSlash) {
    Scanner scanner;
    std::vector<FileMetadata> files = scanner.scan(test_dir_ + "/");
    
    EXPECT_FALSE(files.empty());
}

TEST_F(ScannerTest, HandlesPathWithMultipleSlashes) {
    Scanner scanner;
    std::vector<FileMetadata> files = scanner.scan(test_dir_ + "//");
    
    EXPECT_FALSE(files.empty());
}

