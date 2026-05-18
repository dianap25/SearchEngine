// Authors: Alesia Filinkova, Diana Pelin


#include <gtest/gtest.h>
#include "Database.h"
#include "Indexer.h"
#include "Repository.h"

#include <cstdio>
#include <string>
#include <vector>
#include <algorithm>

class IndexerTest : public ::testing::Test {
protected:
    void SetUp() override {
        ASSERT_TRUE(database.open(":memory:"));
        ASSERT_TRUE(database.initializeSchema());
        
        repository = std::make_unique<Repository>(database.connection());
        indexer = std::make_unique<Indexer>(*repository);
    }

    void TearDown() override {
        indexer.reset();
        repository.reset();
    }

    int saveTestFile(const std::string& path, const std::string& content) {
        FileMetadata metadata;
        metadata.path = path;
        metadata.name = path;
        metadata.extension = ".txt";
        metadata.size = content.size();
        metadata.modified_time = 100;
        
        int file_id = repository->saveFileMetadata(metadata);
        if (file_id > 0) {
            repository->saveFileText(file_id, content);
        }
        return file_id;
    }

    Database database;
    std::unique_ptr<Repository> repository;
    std::unique_ptr<Indexer> indexer;
};

TEST_F(IndexerTest, TokenizesSimpleText) {
    std::vector<std::pair<std::string, int>> tokens = indexer->tokenize("hello world");
    
    ASSERT_EQ(tokens.size(), 2);
    EXPECT_EQ(tokens[0].first, "hello");
    EXPECT_EQ(tokens[0].second, 0);
    EXPECT_EQ(tokens[1].first, "world");
    EXPECT_EQ(tokens[1].second, 1);
}

TEST_F(IndexerTest, TokenizesAndNormalizesContent) {
    std::vector<std::pair<std::string, int>> tokens = indexer->tokenize("Hello, WORLD! hello.");
    
    ASSERT_EQ(tokens.size(), 3);
    EXPECT_EQ(tokens[0].first, "hello");
    EXPECT_EQ(tokens[0].second, 0);
    EXPECT_EQ(tokens[1].first, "world");
    EXPECT_EQ(tokens[1].second, 1);
    EXPECT_EQ(tokens[2].first, "hello");
    EXPECT_EQ(tokens[2].second, 2);
}

TEST_F(IndexerTest, TokenizesWithNumbers) {
    std::vector<std::pair<std::string, int>> tokens = indexer->tokenize("abc123 456def 789");
    
    ASSERT_EQ(tokens.size(), 3);
    EXPECT_EQ(tokens[0].first, "abc123");
    EXPECT_EQ(tokens[1].first, "456def");
    EXPECT_EQ(tokens[2].first, "789");
}

TEST_F(IndexerTest, HandlesEmptyString) {
    std::vector<std::pair<std::string, int>> tokens = indexer->tokenize("");
    
    EXPECT_TRUE(tokens.empty());
}

TEST_F(IndexerTest, HandlesOnlyPunctuation) {
    std::vector<std::pair<std::string, int>> tokens = indexer->tokenize("!@#$%^&*()");
    
    EXPECT_TRUE(tokens.empty());
}

TEST_F(IndexerTest, TokenizesMultipleSpaces) {
    std::vector<std::pair<std::string, int>> tokens = indexer->tokenize("hello    world     test");
    
    ASSERT_EQ(tokens.size(), 3);
    EXPECT_EQ(tokens[0].first, "hello");
    EXPECT_EQ(tokens[1].first, "world");
    EXPECT_EQ(tokens[2].first, "test");
}

TEST_F(IndexerTest, TokenizesWithNewlines) {
    std::vector<std::pair<std::string, int>> tokens = indexer->tokenize("hello\nworld\n\ntest");
    
    ASSERT_EQ(tokens.size(), 3);
    EXPECT_EQ(tokens[0].first, "hello");
    EXPECT_EQ(tokens[1].first, "world");
    EXPECT_EQ(tokens[2].first, "test");
}

TEST_F(IndexerTest, TokenizesWithTabs) {
    std::vector<std::pair<std::string, int>> tokens = indexer->tokenize("hello\tworld\ttest");
    
    ASSERT_EQ(tokens.size(), 3);
    EXPECT_EQ(tokens[0].first, "hello");
    EXPECT_EQ(tokens[1].first, "world");
    EXPECT_EQ(tokens[2].first, "test");
}

TEST_F(IndexerTest, PreservesPositions) {
    std::vector<std::pair<std::string, int>> tokens = indexer->tokenize("a b c d e");
    
    ASSERT_EQ(tokens.size(), 5);
    for (int i = 0; i < 5; ++i) {
        EXPECT_EQ(tokens[i].second, i);
    }
}

TEST_F(IndexerTest, NormalizesToLowerCase) {
    std::vector<std::pair<std::string, int>> tokens = indexer->tokenize("HELLO WoRlD");
    
    ASSERT_EQ(tokens.size(), 2);
    EXPECT_EQ(tokens[0].first, "hello");
    EXPECT_EQ(tokens[1].first, "world");
}

TEST_F(IndexerTest, RemovesPunctuation) {
    std::vector<std::pair<std::string, int>> tokens = indexer->tokenize("hello, world! test?");
    EXPECT_GT(tokens.size(), 0);
    
    for (const auto& token : tokens) {
        if (!token.first.empty()) {
            for (char c : token.first) {
                EXPECT_TRUE(std::isalnum(static_cast<unsigned char>(c)));
            }
            break;
        }
    }
}

TEST_F(IndexerTest, HandlesApostrophes) {
    std::vector<std::pair<std::string, int>> tokens = indexer->tokenize("don't can't it's");
    
    ASSERT_EQ(tokens.size(), 3);
    EXPECT_EQ(tokens[0].first, "dont");
    EXPECT_EQ(tokens[1].first, "cant");
    EXPECT_EQ(tokens[2].first, "its");
}

TEST_F(IndexerTest, HandlesHyphenatedWords) {
    std::vector<std::pair<std::string, int>> tokens = indexer->tokenize("state-of-the-art well-known");
    
    ASSERT_EQ(tokens.size(), 2);
    EXPECT_EQ(tokens[0].first, "stateoftheart");
    EXPECT_EQ(tokens[1].first, "wellknown");
}

TEST_F(IndexerTest, IndexesAllPostingsForRepeatedWord) {
    int file_id = saveTestFile("many.txt", "");
    ASSERT_GT(file_id, 0);
    
    std::string content;
    for (int i = 0; i < 1000; ++i) {
        content += "echo ";
    }
    
    ASSERT_TRUE(indexer->indexFile(file_id, content));
    
    std::vector<SearchResult> results = repository->searchByContent("echo");
    
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].occurrences, 1000);
}

TEST_F(IndexerTest, SavesTokensToDatabase) {
    int file_id = saveTestFile("sample.txt", "hello world hello");
    ASSERT_GT(file_id, 0);
    
    ASSERT_TRUE(indexer->indexFile(file_id, "hello world hello"));
    
    std::vector<SearchResult> results = repository->searchByContent("hello");
    
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].name, "sample.txt");
    EXPECT_EQ(results[0].occurrences, 2);
}

TEST_F(IndexerTest, IndexesMultipleFiles) {
    int file1_id = saveTestFile("file1.txt", "cat dog cat");
    int file2_id = saveTestFile("file2.txt", "dog bird dog");
    ASSERT_GT(file1_id, 0);
    ASSERT_GT(file2_id, 0);
    
    ASSERT_TRUE(indexer->indexFile(file1_id, "cat dog cat"));
    ASSERT_TRUE(indexer->indexFile(file2_id, "dog bird dog"));
    
    std::vector<SearchResult> cat_results = repository->searchByContent("cat");
    ASSERT_EQ(cat_results.size(), 1);
    EXPECT_EQ(cat_results[0].occurrences, 2);
    
    std::vector<SearchResult> dog_results = repository->searchByContent("dog");
    ASSERT_EQ(dog_results.size(), 2);
    
    int total_dog_occurrences = 0;
    for (const auto& result : dog_results) {
        total_dog_occurrences += result.occurrences;
    }
    EXPECT_EQ(total_dog_occurrences, 3);
}

TEST_F(IndexerTest, ReindexingReplacesPostings) {
    int file_id = saveTestFile("reindex.txt", "first version");
    ASSERT_GT(file_id, 0);
    
    ASSERT_TRUE(indexer->indexFile(file_id, "first version"));
    
    std::vector<SearchResult> first_results = repository->searchByContent("first");
    ASSERT_EQ(first_results.size(), 1);
    
    // Reindex with different content
    ASSERT_TRUE(indexer->indexFile(file_id, "second version different"));
    
    std::vector<SearchResult> second_results = repository->searchByContent("first");
    EXPECT_TRUE(second_results.empty());
    
    std::vector<SearchResult> new_results = repository->searchByContent("second");
    ASSERT_EQ(new_results.size(), 1);
}

TEST_F(IndexerTest, IndexesLargeContent) {
    int file_id = saveTestFile("large.txt", "");
    ASSERT_GT(file_id, 0);
    
    std::string content;
    for (int i = 0; i < 10000; ++i) {
        content += "word ";
    }
    
    ASSERT_TRUE(indexer->indexFile(file_id, content));
    
    std::vector<SearchResult> results = repository->searchByContent("word");
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].occurrences, 10000);
}

TEST_F(IndexerTest, HandlesIndexingWithoutText) {
    FileMetadata metadata;
    metadata.path = "notext.txt";
    metadata.name = "notext.txt";
    metadata.extension = ".txt";
    
    int file_id = repository->saveFileMetadata(metadata);
    ASSERT_GT(file_id, 0);

    ASSERT_TRUE(indexer->indexFile(file_id, ""));
    
    std::vector<SearchResult> results = repository->searchByContent("anything");
    EXPECT_TRUE(results.empty());
}

TEST_F(IndexerTest, SearchIsCaseInsensitive) {
    int file_id = saveTestFile("case.txt", "Hello World");
    ASSERT_GT(file_id, 0);
    
    repository->saveFileText(file_id, "Hello World");
    ASSERT_TRUE(indexer->indexFile(file_id, "Hello World"));
    
    std::vector<SearchResult> results_lower = repository->searchByContent("hello");
    std::vector<SearchResult> results_upper = repository->searchByContent("HELLO");
    SUCCEED();
}

TEST_F(IndexerTest, SearchForPartialWordDoesNotMatch) {
    int file_id = saveTestFile("partial.txt", "hello world");
    ASSERT_GT(file_id, 0);
    
    ASSERT_TRUE(indexer->indexFile(file_id, "hello world"));
    
    std::vector<SearchResult> results = repository->searchByContent("hel");
    EXPECT_TRUE(results.empty());
}

TEST_F(IndexerTest, SearchForNonExistentTerm) {
    int file_id = saveTestFile("nonexistent.txt", "hello world");
    ASSERT_GT(file_id, 0);
    
    ASSERT_TRUE(indexer->indexFile(file_id, "hello world"));
    
    std::vector<SearchResult> results = repository->searchByContent("xyzabc");
    EXPECT_TRUE(results.empty());
}

TEST_F(IndexerTest, HandlesVeryLongToken) {
    std::string long_token(10000, 'a');
    std::string content = long_token + " " + long_token;
    
    int file_id = saveTestFile("long.txt", content);
    ASSERT_GT(file_id, 0);
    
    ASSERT_TRUE(indexer->indexFile(file_id, content));
    
    std::vector<SearchResult> results = repository->searchByContent(long_token);
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].occurrences, 2);
}

TEST_F(IndexerTest, HandlesSpecialCharactersInToken) {
    int file_id = saveTestFile("special.txt", "c++ c# python3.8");
    ASSERT_GT(file_id, 0);
    
    ASSERT_TRUE(indexer->indexFile(file_id, "c++ c# python3.8"));
    
    std::vector<SearchResult> results_cpp = repository->searchByContent("c");
    EXPECT_GT(results_cpp.size(), 0);
}

TEST_F(IndexerTest, HandlesUnicodeCharacters) {
    int file_id = saveTestFile("unicode.txt", "zażółć gęślą jaźń");
    ASSERT_GT(file_id, 0);
    
    ASSERT_TRUE(indexer->indexFile(file_id, "zażółć gęślą jaźń"));
    
    std::vector<SearchResult> results = repository->searchByContent("zażółć");
    SUCCEED();
}

TEST_F(IndexerTest, IndexFileDeletesOldPostingsFirst) {
    int file_id = saveTestFile("delete_test.txt", "first content");
    ASSERT_GT(file_id, 0);
    
    ASSERT_TRUE(indexer->indexFile(file_id, "first content"));
    
    std::vector<SearchResult> before = repository->searchByContent("first");
    EXPECT_EQ(before.size(), 1);
    
    ASSERT_TRUE(indexer->indexFile(file_id, "second content"));
    
    std::vector<SearchResult> after_first = repository->searchByContent("first");
    EXPECT_TRUE(after_first.empty());
    
    std::vector<SearchResult> after_second = repository->searchByContent("second");
    EXPECT_EQ(after_second.size(), 1);
}

TEST_F(IndexerTest, TokenizeAndIndexConsistency) {
    std::string content = "the quick brown fox jumps over the lazy dog";
    
    auto tokens = indexer->tokenize(content);
    
    int file_id = saveTestFile("consistency.txt", content);
    ASSERT_GT(file_id, 0);
    ASSERT_TRUE(indexer->indexFile(file_id, content));
    
    std::unordered_map<std::string, int> token_counts;
    for (const auto& [term, pos] : tokens) {
        token_counts[term]++;
    }
    
    for (const auto& [term, count] : token_counts) {
        std::vector<SearchResult> results = repository->searchByContent(term);
        if (count > 0) {
            ASSERT_EQ(results.size(), 1) << "Term: " << term;
            EXPECT_EQ(results[0].occurrences, count) << "Term: " << term;
        }
    }
}
