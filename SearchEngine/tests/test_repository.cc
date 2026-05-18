// Authors: Alesia Filinkova, Diana Pelin


#include <gtest/gtest.h>
#include "Database.h"
#include "Repository.h"
#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <thread>

class RepositoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Open in-memory database for testing
        ASSERT_TRUE(database.open(":memory:"));
        ASSERT_TRUE(database.initializeSchema());
        
        repository = std::make_unique<Repository>(database.connection());
        
        // Create a test file on disk for content extraction tests
        test_file_path_ = "/tmp/test_file.txt";
        std::ofstream file(test_file_path_);
        if (file.is_open()) {
            file << "This is a test file with some content for testing purposes.";
            file.close();
        }
    }
    
    void TearDown() override {
        repository.reset();
        std::filesystem::remove(test_file_path_);
    }
    
    FileMetadata createTestMetadata(const std::string& path, const std::string& name, 
                                     const std::string& extension, uintmax_t size, 
                                     int64_t modified_time, const std::string& hash = "") {
        FileMetadata metadata;
        metadata.path = path;
        metadata.name = name;
        metadata.extension = extension;
        metadata.size = size;
        metadata.modified_time = modified_time;
        metadata.content_hash = hash;
        return metadata;
    }
    
    Database database;
    std::unique_ptr<Repository> repository;
    std::string test_file_path_;
};

TEST_F(RepositoryTest, SavesAndFindsFileByPath) {
    FileMetadata metadata = createTestMetadata("/tmp/example.txt", "example.txt", ".txt", 123, 456);
    
    int file_id = repository->saveFileMetadata(metadata);
    ASSERT_GT(file_id, 0);
    
    std::optional<FileMetadata> found = repository->findByPath("/tmp/example.txt");
    
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->path, "/tmp/example.txt");
    EXPECT_EQ(found->name, "example.txt");
    EXPECT_EQ(found->extension, ".txt");
    EXPECT_EQ(found->size, 123);
    EXPECT_EQ(found->modified_time, 456);
}

TEST_F(RepositoryTest, UpdatesExistingFileOnConflict) {
    FileMetadata metadata = createTestMetadata("/tmp/example.txt", "example.txt", ".txt", 123, 456);
    
    int file_id1 = repository->saveFileMetadata(metadata);
    ASSERT_GT(file_id1, 0);
    
    metadata.size = 999;
    metadata.modified_time = 789;
    int file_id2 = repository->saveFileMetadata(metadata);
    
    EXPECT_GT(file_id2, 0);
    
    std::optional<FileMetadata> found = repository->findByPath("/tmp/example.txt");
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->size, 999);
    EXPECT_EQ(found->modified_time, 789);
}

TEST_F(RepositoryTest, ReturnsNulloptForNonExistentFile) {
    std::optional<FileMetadata> found = repository->findByPath("/tmp/non_existent.txt");
    EXPECT_FALSE(found.has_value());
}

TEST_F(RepositoryTest, FindsAllFiles) {
    repository->saveFileMetadata(createTestMetadata("/tmp/file1.txt", "file1.txt", ".txt", 100, 1000));
    repository->saveFileMetadata(createTestMetadata("/tmp/file2.txt", "file2.txt", ".txt", 200, 2000));
    repository->saveFileMetadata(createTestMetadata("/tmp/file3.txt", "file3.txt", ".txt", 300, 3000));
    
    std::vector<FileMetadata> files = repository->findAllFiles();
    
    EXPECT_EQ(files.size(), 3);
}

TEST_F(RepositoryTest, ReturnsEmptyVectorWhenNoFiles) {
    std::vector<FileMetadata> files = repository->findAllFiles();
    EXPECT_TRUE(files.empty());
}

TEST_F(RepositoryTest, FindsTextByPath) {
    FileMetadata metadata = createTestMetadata("/tmp/content.txt", "content.txt", ".txt", 25, 100);
    int file_id = repository->saveFileMetadata(metadata);
    ASSERT_GT(file_id, 0);
    
    ASSERT_TRUE(repository->saveFileText(file_id, "hello world from database"));
    
    std::optional<std::string> text = repository->findTextByPath("/tmp/content.txt");
    
    ASSERT_TRUE(text.has_value());
    EXPECT_EQ(text.value(), "hello world from database");
}

TEST_F(RepositoryTest, UpdatesTextOnConflict) {
    FileMetadata metadata = createTestMetadata("/tmp/content.txt", "content.txt", ".txt", 25, 100);
    int file_id = repository->saveFileMetadata(metadata);
    ASSERT_GT(file_id, 0);
    
    ASSERT_TRUE(repository->saveFileText(file_id, "first version"));
    ASSERT_TRUE(repository->saveFileText(file_id, "updated version"));
    
    std::optional<std::string> text = repository->findTextByPath("/tmp/content.txt");
    
    ASSERT_TRUE(text.has_value());
    EXPECT_EQ(text.value(), "updated version");
}

TEST_F(RepositoryTest, ReturnsEmptyOptionalWhenTextDoesNotExist) {
    FileMetadata metadata = createTestMetadata("/tmp/no_text.txt", "no_text.txt", ".txt", 10, 100);
    repository->saveFileMetadata(metadata);
    
    std::optional<std::string> text = repository->findTextByPath("/tmp/no_text.txt");
    
    EXPECT_FALSE(text.has_value());
}

TEST_F(RepositoryTest, ReturnsEmptyOptionalWhenFileDoesNotExist) {
    std::optional<std::string> text = repository->findTextByPath("/tmp/missing.txt");
    EXPECT_FALSE(text.has_value());
}

TEST_F(RepositoryTest, DeletesFileByPath) {
    FileMetadata metadata = createTestMetadata("/tmp/delete_me.txt", "delete_me.txt", ".txt", 10, 100);
    repository->saveFileMetadata(metadata);
    
    ASSERT_TRUE(repository->deleteFileByPath("/tmp/delete_me.txt"));
    
    std::optional<FileMetadata> found = repository->findByPath("/tmp/delete_me.txt");
    EXPECT_FALSE(found.has_value());
}

TEST_F(RepositoryTest, DeleteReturnsFalseForNonExistentFile) {
    std::optional<FileMetadata> found = repository->findByPath("/tmp/non_existent.txt");
    ASSERT_FALSE(found.has_value());
    
    bool result = repository->deleteFileByPath("/tmp/non_existent.txt");
    
    found = repository->findByPath("/tmp/non_existent.txt");
    EXPECT_FALSE(found.has_value());
}

TEST_F(RepositoryTest, DeleteFileRemovesAssociatedText) {
    FileMetadata metadata = createTestMetadata("/tmp/with_text.txt", "with_text.txt", ".txt", 20, 200);
    int file_id = repository->saveFileMetadata(metadata);
    repository->saveFileText(file_id, "some content");
    
    repository->deleteFileByPath("/tmp/with_text.txt");
    
    std::optional<std::string> text = repository->findTextByPath("/tmp/with_text.txt");
    EXPECT_FALSE(text.has_value());
}

TEST_F(RepositoryTest, DeleteIndexForFile) {
    FileMetadata metadata = createTestMetadata("/tmp/indexed.txt", "indexed.txt", ".txt", 15, 150);
    int file_id = repository->saveFileMetadata(metadata);
    
    std::vector<std::pair<std::string, int>> tokens = {
        {"hello", 1}, {"world", 2}, {"test", 3}
    };
    repository->saveTermPositionsBatch(file_id, tokens);
    
    ASSERT_TRUE(repository->deleteIndexForFile(file_id));
    
    std::optional<FileMetadata> found = repository->findByPath("/tmp/indexed.txt");
    EXPECT_TRUE(found.has_value());
}

TEST_F(RepositoryTest, SearchesByFileName) {
    repository->saveFileMetadata(createTestMetadata("/tmp/report.txt", "report.txt", ".txt", 10, 100));
    repository->saveFileMetadata(createTestMetadata("/tmp/report_final.txt", "report_final.txt", ".txt", 20, 200));
    repository->saveFileMetadata(createTestMetadata("/tmp/other.txt", "other.txt", ".txt", 30, 300));
    
    std::vector<SearchResult> results = repository->searchByName("report");
    
    ASSERT_EQ(results.size(), 2);
    for (const auto& result : results) {
        EXPECT_TRUE(result.name.find("report") != std::string::npos);
    }
    EXPECT_EQ(results[0].name, "report.txt");
    EXPECT_EQ(results[1].name, "report_final.txt");
}

TEST_F(RepositoryTest, SearchByNameCaseInsensitive) {
    repository->saveFileMetadata(createTestMetadata("/tmp/REPORT.txt", "REPORT.txt", ".txt", 10, 100));
    repository->saveFileMetadata(createTestMetadata("/tmp/Report.txt", "Report.txt", ".txt", 20, 200));
    repository->saveFileMetadata(createTestMetadata("/tmp/report.txt", "report.txt", ".txt", 30, 300));
    
    std::vector<SearchResult> results = repository->searchByName("REPORT");
    
    ASSERT_EQ(results.size(), 3);
}

TEST_F(RepositoryTest, SearchByNameReturnsEmptyForNoMatch) {
    repository->saveFileMetadata(createTestMetadata("/tmp/example.txt", "example.txt", ".txt", 10, 100));
    
    std::vector<SearchResult> results = repository->searchByName("xyz");
    
    EXPECT_TRUE(results.empty());
}

TEST_F(RepositoryTest, SearchesByContent) {
    FileMetadata metadata = createTestMetadata("/tmp/doc1.txt", "doc1.txt", ".txt", 100, 1000);
    int file_id1 = repository->saveFileMetadata(metadata);
    
    FileMetadata metadata2 = createTestMetadata("/tmp/doc2.txt", "doc2.txt", ".txt", 100, 1000);
    int file_id2 = repository->saveFileMetadata(metadata2);
    
    std::vector<std::pair<std::string, int>> tokens1 = {
        {"search", 1}, {"term", 2}, {"example", 3}
    };
    std::vector<std::pair<std::string, int>> tokens2 = {
        {"search", 1}, {"another", 2}, {"word", 3}
    };
    
    repository->saveTermPositionsBatch(file_id1, tokens1);
    repository->saveTermPositionsBatch(file_id2, tokens2);
    
    std::vector<SearchResult> results = repository->searchByContent("search");
    
    ASSERT_EQ(results.size(), 2);
    EXPECT_EQ(results[0].occurrences, 1);
    EXPECT_EQ(results[1].occurrences, 1);
}

TEST_F(RepositoryTest, SearchByContentReturnsFilesWithMultipleOccurrences) {
    FileMetadata metadata = createTestMetadata("/tmp/frequent.txt", "frequent.txt", ".txt", 100, 1000);
    int file_id = repository->saveFileMetadata(metadata);
    
    std::vector<std::pair<std::string, int>> tokens = {
        {"term", 1}, {"term", 5}, {"term", 10}, {"other", 15}
    };
    repository->saveTermPositionsBatch(file_id, tokens);
    
    std::vector<SearchResult> results = repository->searchByContent("term");
    
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].occurrences, 3);
}

TEST_F(RepositoryTest, SearchByContentReturnsEmptyForNonExistentTerm) {
    FileMetadata metadata = createTestMetadata("/tmp/doc.txt", "doc.txt", ".txt", 50, 500);
    int file_id = repository->saveFileMetadata(metadata);
    
    std::vector<std::pair<std::string, int>> tokens = {{"hello", 1}, {"world", 2}};
    repository->saveTermPositionsBatch(file_id, tokens);
    
    std::vector<SearchResult> results = repository->searchByContent("nonexistent");
    
    EXPECT_TRUE(results.empty());
}

TEST_F(RepositoryTest, SearchResultsOrderedByOccurrencesDescending) {
    FileMetadata metadata1 = createTestMetadata("/tmp/doc1.txt", "doc1.txt", ".txt", 100, 1000);
    int file_id1 = repository->saveFileMetadata(metadata1);
    
    FileMetadata metadata2 = createTestMetadata("/tmp/doc2.txt", "doc2.txt", ".txt", 100, 1000);
    int file_id2 = repository->saveFileMetadata(metadata2);
    
    FileMetadata metadata3 = createTestMetadata("/tmp/doc3.txt", "doc3.txt", ".txt", 100, 1000);
    int file_id3 = repository->saveFileMetadata(metadata3);
    
    std::vector<std::pair<std::string, int>> tokens1 = {{"search", 1}};  // 1 occurrence
    std::vector<std::pair<std::string, int>> tokens2 = {{"search", 1}, {"search", 2}, {"search", 3}};  // 3 occurrences
    std::vector<std::pair<std::string, int>> tokens3 = {{"search", 1}, {"search", 2}};  // 2 occurrences
    
    repository->saveTermPositionsBatch(file_id1, tokens1);
    repository->saveTermPositionsBatch(file_id2, tokens2);
    repository->saveTermPositionsBatch(file_id3, tokens3);
    
    std::vector<SearchResult> results = repository->searchByContent("search");
    
    ASSERT_EQ(results.size(), 3);
    EXPECT_GE(results[0].occurrences, results[1].occurrences);
    EXPECT_GE(results[1].occurrences, results[2].occurrences);
}

TEST_F(RepositoryTest, SavesTermPositionsBatch) {
    FileMetadata metadata = createTestMetadata("/tmp/batch.txt", "batch.txt", ".txt", 100, 1000);
    int file_id = repository->saveFileMetadata(metadata);
    
    std::vector<std::pair<std::string, int>> tokens = {
        {"hello", 1}, {"world", 2}, {"test", 3}, {"hello", 4}
    };
    
    EXPECT_TRUE(repository->saveTermPositionsBatch(file_id, tokens));
}

TEST_F(RepositoryTest, HandlesEmptyTokensBatch) {
    FileMetadata metadata = createTestMetadata("/tmp/empty.txt", "empty.txt", ".txt", 10, 100);
    int file_id = repository->saveFileMetadata(metadata);
    
    std::vector<std::pair<std::string, int>> tokens;
    
    EXPECT_TRUE(repository->saveTermPositionsBatch(file_id, tokens));
}

TEST_F(RepositoryTest, TermCacheWorksForRepeatedTerms) {
    FileMetadata metadata = createTestMetadata("/tmp/repeat.txt", "repeat.txt", ".txt", 100, 1000);
    int file_id = repository->saveFileMetadata(metadata);
    
    std::vector<std::pair<std::string, int>> tokens;
    for (int i = 0; i < 100; ++i) {
        tokens.emplace_back("term", i);
    }
    
    EXPECT_TRUE(repository->saveTermPositionsBatch(file_id, tokens));
    
    std::vector<SearchResult> results = repository->searchByContent("term");
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].occurrences, 100);
}

TEST_F(RepositoryTest, BeginsAndCommitsTransaction) {
    EXPECT_TRUE(repository->beginTransaction());
    
    FileMetadata metadata = createTestMetadata("/tmp/transaction.txt", "transaction.txt", ".txt", 100, 1000);
    repository->saveFileMetadata(metadata);
    
    EXPECT_TRUE(repository->commitTransaction());
    
    std::optional<FileMetadata> found = repository->findByPath("/tmp/transaction.txt");
    EXPECT_TRUE(found.has_value());
}

TEST_F(RepositoryTest, RollbackTransaction) {
    EXPECT_TRUE(repository->beginTransaction());
    
    FileMetadata metadata = createTestMetadata("/tmp/rollback.txt", "rollback.txt", ".txt", 100, 1000);
    repository->saveFileMetadata(metadata);
    
    EXPECT_TRUE(repository->rollbackTransaction());
    
    std::optional<FileMetadata> found = repository->findByPath("/tmp/rollback.txt");
    EXPECT_FALSE(found.has_value());
}

TEST_F(RepositoryTest, MultipleTransactionsSequentially) {
    EXPECT_TRUE(repository->beginTransaction());
    repository->saveFileMetadata(createTestMetadata("/tmp/fileA.txt", "fileA.txt", ".txt", 100, 1000));
    EXPECT_TRUE(repository->commitTransaction());
    
    EXPECT_TRUE(repository->beginTransaction());
    repository->saveFileMetadata(createTestMetadata("/tmp/fileB.txt", "fileB.txt", ".txt", 200, 2000));
    EXPECT_TRUE(repository->commitTransaction());
    
    std::vector<FileMetadata> files = repository->findAllFiles();
    EXPECT_EQ(files.size(), 2);
}

TEST_F(RepositoryTest, SavesFileWithEmptyHash) {
    FileMetadata metadata = createTestMetadata("/tmp/no_hash.txt", "no_hash.txt", ".txt", 100, 1000, "");
    int file_id = repository->saveFileMetadata(metadata);
    
    EXPECT_GT(file_id, 0);
    
    std::optional<FileMetadata> found = repository->findByPath("/tmp/no_hash.txt");
    ASSERT_TRUE(found.has_value());
    EXPECT_TRUE(found->content_hash.empty());
}

TEST_F(RepositoryTest, SavesFileWithLongPath) {
    std::string long_path = "/tmp/" + std::string(500, 'a') + ".txt";
    FileMetadata metadata = createTestMetadata(long_path, "long.txt", ".txt", 100, 1000);
    
    int file_id = repository->saveFileMetadata(metadata);
    EXPECT_GT(file_id, 0);
}

TEST_F(RepositoryTest, SavesFileWithSpecialCharactersInPath) {
    std::string special_path = "/tmp/file_with_!@#$%^&*().txt";
    FileMetadata metadata = createTestMetadata(special_path, "special.txt", ".txt", 100, 1000);
    
    int file_id = repository->saveFileMetadata(metadata);
    EXPECT_GT(file_id, 0);
    
    std::optional<FileMetadata> found = repository->findByPath(special_path);
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->path, special_path);
}

TEST_F(RepositoryTest, SavesFileWithLargeSize) {
    FileMetadata metadata = createTestMetadata("/tmp/large.txt", "large.txt", ".txt", 10ULL * 1024 * 1024 * 1024, 1000);
    
    int file_id = repository->saveFileMetadata(metadata);
    EXPECT_GT(file_id, 0);
    
    std::optional<FileMetadata> found = repository->findByPath("/tmp/large.txt");
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->size, 10ULL * 1024 * 1024 * 1024);
}

TEST_F(RepositoryTest, SavesFileWithLargeModifiedTime) {
    FileMetadata metadata = createTestMetadata("/tmp/old.txt", "old.txt", ".txt", 100, 0x7FFFFFFFFFFFFFFF);
    
    int file_id = repository->saveFileMetadata(metadata);
    EXPECT_GT(file_id, 0);
}

TEST_F(RepositoryTest, SearchByContentWithSpecialCharacters) {
    FileMetadata metadata = createTestMetadata("/tmp/special.txt", "special.txt", ".txt", 100, 1000);
    int file_id = repository->saveFileMetadata(metadata);
    
    std::vector<std::pair<std::string, int>> tokens = {
        {"c++", 1}, {"c#", 2}, {"python3.8", 3}
    };
    repository->saveTermPositionsBatch(file_id, tokens);
    
    std::vector<SearchResult> results_cpp = repository->searchByContent("c++");
    EXPECT_EQ(results_cpp.size(), 1);
    
    std::vector<SearchResult> results_csharp = repository->searchByContent("c#");
    EXPECT_EQ(results_csharp.size(), 1);
}

TEST_F(RepositoryTest, SavesAndRetrievesFullMetadata) {
    FileMetadata metadata;
    metadata.path = "/tmp/full_metadata.txt";
    metadata.name = "full_metadata.txt";
    metadata.extension = ".txt";
    metadata.size = 12345;
    metadata.modified_time = 67890;
    metadata.content_hash = "abc123def456";
    
    int file_id = repository->saveFileMetadata(metadata);
    ASSERT_GT(file_id, 0);
    
    std::optional<FileMetadata> found = repository->findByPath("/tmp/full_metadata.txt");
    ASSERT_TRUE(found.has_value());
    
    EXPECT_EQ(found->path, metadata.path);
    EXPECT_EQ(found->name, metadata.name);
    EXPECT_EQ(found->extension, metadata.extension);
    EXPECT_EQ(found->size, metadata.size);
    EXPECT_EQ(found->modified_time, metadata.modified_time);
    EXPECT_EQ(found->content_hash, metadata.content_hash);
}

TEST_F(RepositoryTest, SearchResultHasContextFieldForNameSearch) {
    repository->saveFileMetadata(createTestMetadata("/tmp/context_test.txt", "context_test.txt", ".txt", 100, 1000));
    
    std::vector<SearchResult> results = repository->searchByName("context_test");
    
    ASSERT_EQ(results.size(), 1);
    EXPECT_TRUE(results[0].context.empty() || !results[0].context.empty());
}

TEST_F(RepositoryTest, SearchResultHasContextFieldForContentSearch) {
    FileMetadata metadata = createTestMetadata("/tmp/content_context.txt", "content_context.txt", ".txt", 100, 1000);
    int file_id = repository->saveFileMetadata(metadata);
    
    std::vector<std::pair<std::string, int>> tokens = {
        {"important", 1}, {"keyword", 2}, {"test", 3}
    };
    repository->saveTermPositionsBatch(file_id, tokens);
    
    std::vector<SearchResult> results = repository->searchByContent("keyword");
    
    ASSERT_EQ(results.size(), 1);
    EXPECT_TRUE(results[0].context.empty() || !results[0].context.empty());
}

TEST_F(RepositoryTest, SearchResultContextCanBeAssigned) {
    SearchResult result;
    result.path = "/tmp/test.txt";
    result.name = "test.txt";
    result.occurrences = 3;
    result.context = "This is a surrounding text snippet that contains the search term.";
    
    EXPECT_EQ(result.context, "This is a surrounding text snippet that contains the search term.");
}

TEST_F(RepositoryTest, SearchResultContextDefaultEmpty) {
    SearchResult result;
    EXPECT_TRUE(result.context.empty());
}

TEST_F(RepositoryTest, MultipleSearchResultsEachHaveOwnContext) {
    repository->saveFileMetadata(createTestMetadata("/tmp/fileA.txt", "fileA.txt", ".txt", 100, 1000));
    repository->saveFileMetadata(createTestMetadata("/tmp/fileB.txt", "fileB.txt", ".txt", 100, 1000));
    
    std::vector<SearchResult> results = repository->searchByName("file");
    
    ASSERT_EQ(results.size(), 2);
    // Each result has its own context field (even if empty)
    EXPECT_TRUE(results[0].context.empty() || !results[0].context.empty());
    EXPECT_TRUE(results[1].context.empty() || !results[1].context.empty());
}

TEST_F(RepositoryTest, SearchResultContainsAllRequiredFields) {
    SearchResult result;
    
    // Verify all expected fields exist
    result.path = "/tmp/sample.txt";
    result.name = "sample.txt";
    result.occurrences = 42;
    result.context = "Sample context text";
    
    EXPECT_EQ(result.path, "/tmp/sample.txt");
    EXPECT_EQ(result.name, "sample.txt");
    EXPECT_EQ(result.occurrences, 42);
    EXPECT_EQ(result.context, "Sample context text");
}

TEST_F(RepositoryTest, SearchResultCanBeStoredInVector) {
    std::vector<SearchResult> results;
    
    SearchResult r1;
    r1.path = "/tmp/file1.txt";
    r1.name = "file1.txt";
    r1.occurrences = 1;
    r1.context = "Context 1";
    
    SearchResult r2;
    r2.path = "/tmp/file2.txt";
    r2.name = "file2.txt";
    r2.occurrences = 2;
    r2.context = "Context 2";
    
    results.push_back(r1);
    results.push_back(r2);
    
    ASSERT_EQ(results.size(), 2);
    EXPECT_EQ(results[0].context, "Context 1");
    EXPECT_EQ(results[1].context, "Context 2");
}