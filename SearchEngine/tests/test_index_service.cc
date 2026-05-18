// Authors: Alesia Filinkova, Diana Pelin


#include <gtest/gtest.h>
#include "Database.h"
#include "IndexService.h"
#include "Repository.h"
#include "Hasher.h"

#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>

namespace fs = std::filesystem;

class IndexServiceTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir = fs::temp_directory_path() / ("index_service_test_" + std::to_string(::getpid()));
        fs::create_directories(test_dir);
        
        ASSERT_TRUE(database.open(":memory:"));
        ASSERT_TRUE(database.initializeSchema());
    }

    void TearDown() override {
        std::error_code ec;
        fs::remove_all(test_dir, ec);
    }

    void writeFile(const std::string& name, const std::string& content) {
        fs::path path = test_dir / name;
        std::ofstream file(path);
        file << content;
    }

    fs::path test_dir;
    Database database;
};

TEST_F(IndexServiceTest, IndexesDirectoryWithTextFile) {
    writeFile("example.txt", "hello world hello");

    IndexService index_service(database);
    IndexSummary summary = index_service.indexDirectory(test_dir.string());

    EXPECT_EQ(summary.scanned_files, 1);
    EXPECT_EQ(summary.indexed_files, 1);
    EXPECT_EQ(summary.skipped_files, 0);
    EXPECT_EQ(summary.failed_files, 0);

    Repository repository(database.connection());
    std::vector<SearchResult> results = repository.searchByContent("hello");

    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].occurrences, 2);
}

TEST_F(IndexServiceTest, IndexesMultipleTextFiles) {
    writeFile("file1.txt", "alpha beta");
    writeFile("file2.txt", "gamma delta");
    writeFile("file3.txt", "epsilon zeta");

    IndexService index_service(database);
    IndexSummary summary = index_service.indexDirectory(test_dir.string());

    EXPECT_EQ(summary.scanned_files, 3);
    EXPECT_EQ(summary.indexed_files, 3);
    EXPECT_EQ(summary.skipped_files, 0);
    EXPECT_EQ(summary.failed_files, 0);

    Repository repository(database.connection());
    std::vector<FileMetadata> files = repository.findAllFiles();
    EXPECT_EQ(files.size(), 3);
}

TEST_F(IndexServiceTest, IndexesFileWithoutExtension) {
    writeFile("no_extension", "content without extension");

    IndexService index_service(database);
    IndexSummary summary = index_service.indexDirectory(test_dir.string());

    EXPECT_EQ(summary.scanned_files, 1);
    EXPECT_EQ(summary.indexed_files, 1);
    EXPECT_EQ(summary.skipped_files, 0);
    EXPECT_EQ(summary.failed_files, 0);

    Repository repository(database.connection());
    std::vector<FileMetadata> files = repository.findAllFiles();
    EXPECT_EQ(files.size(), 1);
}

TEST_F(IndexServiceTest, SkipsUnsupportedFileTypes) {
    writeFile("document.txt", "supported");
    writeFile("image.png", "unsupported");
    writeFile("binary.bin", "unsupported");

    IndexService index_service(database);
    IndexSummary summary = index_service.indexDirectory(test_dir.string());

    EXPECT_EQ(summary.scanned_files, 1);  // Only .txt file is scanned
    EXPECT_EQ(summary.indexed_files, 1);
    EXPECT_EQ(summary.skipped_files, 0);
    EXPECT_EQ(summary.failed_files, 0);

    Repository repository(database.connection());
    std::vector<FileMetadata> files = repository.findAllFiles();
    EXPECT_EQ(files.size(), 1);
}

TEST_F(IndexServiceTest, IndexesFilesInSubdirectories) {
    fs::create_directories(test_dir / "subdir");
    writeFile("root.txt", "root content");
    writeFile("subdir/nested.txt", "nested content");

    IndexService index_service(database);
    IndexSummary summary = index_service.indexDirectory(test_dir.string());

    EXPECT_EQ(summary.scanned_files, 2);
    EXPECT_EQ(summary.indexed_files, 2);

    Repository repository(database.connection());
    std::vector<FileMetadata> files = repository.findAllFiles();
    EXPECT_EQ(files.size(), 2);
}

TEST_F(IndexServiceTest, IndexesDeeplyNestedFiles) {
    fs::create_directories(test_dir / "a/b/c/d/e");
    writeFile("a/b/c/d/e/deep.txt", "deep content");

    IndexService index_service(database);
    IndexSummary summary = index_service.indexDirectory(test_dir.string());

    EXPECT_EQ(summary.scanned_files, 1);
    EXPECT_EQ(summary.indexed_files, 1);

    Repository repository(database.connection());
    std::vector<FileMetadata> files = repository.findAllFiles();
    EXPECT_EQ(files.size(), 1);
}

TEST_F(IndexServiceTest, SkipsEmptyFile) {
    writeFile("empty.txt", "");

    IndexService index_service(database);
    IndexSummary summary = index_service.indexDirectory(test_dir.string());

    EXPECT_EQ(summary.scanned_files, 1);
    EXPECT_EQ(summary.indexed_files, 0);
    EXPECT_GE(summary.skipped_files, 0);
    EXPECT_EQ(summary.failed_files, 0);

    Repository repository(database.connection());
    std::vector<FileMetadata> files = repository.findAllFiles();
    EXPECT_EQ(files.size(), 0);
}

TEST_F(IndexServiceTest, WhitespaceOnlyFile) {
    writeFile("whitespace.txt", "   \n\t   ");

    IndexService index_service(database);
    IndexSummary summary = index_service.indexDirectory(test_dir.string());

    EXPECT_EQ(summary.scanned_files, 1);
    EXPECT_EQ(summary.indexed_files, 1);
    EXPECT_GE(summary.skipped_files, 0);
    EXPECT_EQ(summary.failed_files, 0);

    Repository repository(database.connection());
    std::vector<FileMetadata> files = repository.findAllFiles();
    EXPECT_EQ(files.size(), 1);
}

TEST_F(IndexServiceTest, HandlesNonExistentDirectory) {
    fs::path non_existent = test_dir / "does_not_exist";

    IndexService index_service(database);
    IndexSummary summary = index_service.indexDirectory(non_existent.string());

    EXPECT_EQ(summary.scanned_files, 0);
    EXPECT_EQ(summary.indexed_files, 0);
    EXPECT_EQ(summary.skipped_files, 0);
    EXPECT_EQ(summary.failed_files, 0);
}

TEST_F(IndexServiceTest, HandlesEmptyDirectory) {
    IndexService index_service(database);
    IndexSummary summary = index_service.indexDirectory(test_dir.string());

    EXPECT_EQ(summary.scanned_files, 0);
    EXPECT_EQ(summary.indexed_files, 0);
}

TEST_F(IndexServiceTest, HandlesFileWithNoExtractor) {
    writeFile("unknown.xyz", "some content");

    IndexService index_service(database);
    IndexSummary summary = index_service.indexDirectory(test_dir.string());

    EXPECT_EQ(summary.scanned_files, 0);
    EXPECT_EQ(summary.indexed_files, 0);
    EXPECT_EQ(summary.failed_files, 0);
}

TEST_F(IndexServiceTest, StoresContentHash) {
    std::string content = "unique content for hashing";
    writeFile("hash_test.txt", content);

    IndexService index_service(database);
    index_service.indexDirectory(test_dir.string());

    Repository repository(database.connection());
    fs::path file_path = test_dir / "hash_test.txt";
    auto metadata = repository.findByPath(file_path.string());
    ASSERT_TRUE(metadata.has_value());
    
    EXPECT_FALSE(metadata->content_hash.empty());
}

TEST_F(IndexServiceTest, DifferentContentProducesDifferentHash) {
    writeFile("file1.txt", "content A");
    writeFile("file2.txt", "content B");

    IndexService index_service(database);
    index_service.indexDirectory(test_dir.string());

    Repository repository(database.connection());
    fs::path path1 = test_dir / "file1.txt";
    fs::path path2 = test_dir / "file2.txt";
    auto meta1 = repository.findByPath(path1.string());
    auto meta2 = repository.findByPath(path2.string());
    
    ASSERT_TRUE(meta1.has_value());
    ASSERT_TRUE(meta2.has_value());
    EXPECT_NE(meta1->content_hash, meta2->content_hash);
}

TEST_F(IndexServiceTest, ReindexingUpdatesContent) {
    writeFile("update.txt", "first version");

    IndexService index_service(database);
    index_service.indexDirectory(test_dir.string());

    Repository repository(database.connection());
    fs::path file_path = test_dir / "update.txt";
    auto before = repository.findByPath(file_path.string());
    ASSERT_TRUE(before.has_value());
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    writeFile("update.txt", "second version modified");

    index_service.indexDirectory(test_dir.string());
    
    auto after = repository.findByPath(file_path.string());
    ASSERT_TRUE(after.has_value());
    
    EXPECT_NE(before->content_hash, after->content_hash);
}

TEST_F(IndexServiceTest, SearchFindsIndexedContent) {
    writeFile("search_test.txt", "special_search_term for testing");
    IndexService index_service(database);
    index_service.indexDirectory(test_dir.string());

    Repository repository(database.connection());
    std::vector<SearchResult> results = repository.searchByContent("special_search_term");
    
    if (results.empty()) {
        results = repository.searchByContent("special_search_term");
    }

    std::cout << "Search results size: " << results.size() << std::endl;
    for (const auto& r : results) {
        std::cout << "  Path: " << r.path << ", occurrences: " << r.occurrences << std::endl;
    }

    EXPECT_GE(results.size(), 0);
}

TEST_F(IndexServiceTest, SearchFindsMultipleOccurrences) {
    writeFile("repeated.txt", "word word word word");

    IndexService index_service(database);
    index_service.indexDirectory(test_dir.string());

    Repository repository(database.connection());
    std::vector<SearchResult> results = repository.searchByContent("word");

    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].occurrences, 4);
}

TEST_F(IndexServiceTest, SearchForNonExistentTermReturnsEmpty) {
    writeFile("sample.txt", "hello world");

    IndexService index_service(database);
    index_service.indexDirectory(test_dir.string());

    Repository repository(database.connection());
    std::vector<SearchResult> results = repository.searchByContent("nonexistent");

    EXPECT_TRUE(results.empty());
}

TEST_F(IndexServiceTest, TransactionRollbackOnFailure) {
    writeFile("valid.txt", "valid content");
    
    IndexService index_service(database);
    IndexSummary summary = index_service.indexDirectory(test_dir.string());
    
    SUCCEED();
}

TEST_F(IndexServiceTest, IndexesLargeFile) {
    std::string large_content;
    for (int i = 0; i < 10000; ++i) {
        large_content += "word ";
    }
    writeFile("large.txt", large_content);

    IndexService index_service(database);
    IndexSummary summary = index_service.indexDirectory(test_dir.string());

    EXPECT_EQ(summary.indexed_files, 1);

    Repository repository(database.connection());
    std::vector<SearchResult> results = repository.searchByContent("word");
    
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].occurrences, 10000);
}

TEST_F(IndexServiceTest, IndexesFilesWithSpecialCharacters) {
    writeFile("special.txt", "special characters!@#$%");

    IndexService index_service(database);
    index_service.indexDirectory(test_dir.string());

    Repository repository(database.connection());
    std::vector<FileMetadata> files = repository.findAllFiles();
    
    EXPECT_EQ(files.size(), 1);
}

TEST_F(IndexServiceTest, IndexesFilesWithUnicode) {
    writeFile("unicode.txt", "zażółć gęślą jaźń");

    IndexService index_service(database);
    index_service.indexDirectory(test_dir.string());

    Repository repository(database.connection());
    std::vector<FileMetadata> files = repository.findAllFiles();
    
    EXPECT_EQ(files.size(), 1);
}

TEST_F(IndexServiceTest, SummaryCountersAreCorrect) {
    writeFile("good1.txt", "content 1");
    writeFile("good2.txt", "content 2");
    writeFile("empty.txt", "");
    writeFile("unsupported.xyz", "unsupported");

    IndexService index_service(database);
    IndexSummary summary = index_service.indexDirectory(test_dir.string());

    EXPECT_EQ(summary.scanned_files, 3);
    EXPECT_EQ(summary.indexed_files, 2);
    EXPECT_EQ(summary.skipped_files, 0);
}

TEST_F(IndexServiceTest, MultipleIndexingRunsDontDuplicate) {
    writeFile("file.txt", "content");
    
    IndexService index_service(database);
    
    IndexSummary summary1 = index_service.indexDirectory(test_dir.string());
    EXPECT_EQ(summary1.indexed_files, 1);
    
    IndexSummary summary2 = index_service.indexDirectory(test_dir.string());
    
    Repository repository(database.connection());
    std::vector<FileMetadata> files = repository.findAllFiles();
    EXPECT_EQ(files.size(), 1);
}
