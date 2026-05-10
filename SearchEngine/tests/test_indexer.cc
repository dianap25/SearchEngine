// Authors: Alesia Filinkova, Diana Pelin
// Description: Unit tests for Indexer. Cover token normalization and
// the end-to-end "save text + index file" flow against an in-memory
// database.

#include <gtest/gtest.h>

#include "Database.h"
#include "Indexer.h"
#include "Repository.h"

#include <cstdio>
#include <string>
#include <vector>

TEST(IndexerTest, TokenizesAndNormalizesContent) {
    Database database;
    ASSERT_TRUE(database.open(":memory:"));
    ASSERT_TRUE(database.initializeSchema());

    Repository repository(database.connection());
    Indexer indexer(repository);

    std::vector<std::pair<std::string, int>> tokens =
        indexer.tokenize("Hello, WORLD! hello.");

    ASSERT_EQ(tokens.size(), 3);

    EXPECT_EQ(tokens[0].first, "hello");
    EXPECT_EQ(tokens[0].second, 0);

    EXPECT_EQ(tokens[1].first, "world");
    EXPECT_EQ(tokens[1].second, 1);

    EXPECT_EQ(tokens[2].first, "hello");
    EXPECT_EQ(tokens[2].second, 2);
}

TEST(IndexerTest, IndexesAllPostingsForRepeatedWord) {
    Database database;
    ASSERT_TRUE(database.open(":memory:"));
    ASSERT_TRUE(database.initializeSchema());

    Repository repository(database.connection());
    Indexer indexer(repository);

    FileMetadata metadata;
    metadata.path = "many.txt";
    metadata.name = "many.txt";
    metadata.extension = ".txt";

    int file_id = repository.saveFileMetadata(metadata);
    ASSERT_GT(file_id, 0);

    std::string content;
    for (int i = 0; i < 1000; ++i) {
        content += "echo ";
    }

    ASSERT_TRUE(indexer.indexFile(file_id, content));

    std::vector<SearchResult> results = repository.searchByContent("echo");

    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].occurrences, 1000);
}

TEST(IndexerTest, SavesTokensToDatabase) {
    Database database;
    ASSERT_TRUE(database.open(":memory:"));
    ASSERT_TRUE(database.initializeSchema());

    Repository repository(database.connection());
    Indexer indexer(repository);

    FileMetadata metadata;
    metadata.path = "sample.txt";
    metadata.name = "sample.txt";
    metadata.extension = ".txt";
    metadata.size = 20;
    metadata.modified_time = 100;

    int file_id = repository.saveFileMetadata(metadata);
    ASSERT_GT(file_id, 0);

    ASSERT_TRUE(repository.saveFileText(file_id, "hello world hello"));
    ASSERT_TRUE(indexer.indexFile(file_id, "hello world hello"));

    std::vector<SearchResult> results = repository.searchByContent("hello");

    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].name, "sample.txt");
    EXPECT_EQ(results[0].occurrences, 2);
}
