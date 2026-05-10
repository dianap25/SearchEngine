// Authors: Alesia Filinkova, Diana Pelin


#include <gtest/gtest.h>

#include "Database.h"
#include "Repository.h"

TEST(RepositoryTest, SavesAndFindsFileByPath) {
    Database database;
    ASSERT_TRUE(database.open(":memory:"));
    ASSERT_TRUE(database.initializeSchema());

    Repository repository(database.connection());

    FileMetadata metadata;
    metadata.path = "/tmp/example.txt";
    metadata.name = "example.txt";
    metadata.extension = ".txt";
    metadata.size = 123;
    metadata.modified_time = 456;

    int file_id = repository.saveFileMetadata(metadata);

    ASSERT_GT(file_id, 0);

    std::optional<FileMetadata> found = repository.findByPath("/tmp/example.txt");

    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->path, "/tmp/example.txt");
    EXPECT_EQ(found->name, "example.txt");
    EXPECT_EQ(found->extension, ".txt");
    EXPECT_EQ(found->size, 123);
    EXPECT_EQ(found->modified_time, 456);
}

TEST(RepositoryTest, SearchesByFileName) {
    Database database;
    ASSERT_TRUE(database.open(":memory:"));
    ASSERT_TRUE(database.initializeSchema());

    Repository repository(database.connection());

    FileMetadata metadata;
    metadata.path = "/tmp/report.txt";
    metadata.name = "report.txt";
    metadata.extension = ".txt";
    metadata.size = 10;
    metadata.modified_time = 100;

    ASSERT_GT(repository.saveFileMetadata(metadata), 0);

    std::vector<SearchResult> results = repository.searchByName("report");

    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].name, "report.txt");
}

TEST(RepositoryTest, DeletesFileByPath) {
    Database database;
    ASSERT_TRUE(database.open(":memory:"));
    ASSERT_TRUE(database.initializeSchema());

    Repository repository(database.connection());

    FileMetadata metadata;
    metadata.path = "/tmp/delete_me.txt";
    metadata.name = "delete_me.txt";
    metadata.extension = ".txt";
    metadata.size = 10;
    metadata.modified_time = 100;

    ASSERT_GT(repository.saveFileMetadata(metadata), 0);

    ASSERT_TRUE(repository.deleteFileByPath("/tmp/delete_me.txt"));

    std::optional<FileMetadata> found = repository.findByPath("/tmp/delete_me.txt");

    EXPECT_FALSE(found.has_value());
}

TEST(RepositoryTest, FindsTextByPath) {
    Database database;
    ASSERT_TRUE(database.open(":memory:"));
    ASSERT_TRUE(database.initializeSchema());

    Repository repository(database.connection());

    FileMetadata metadata;
    metadata.path = "/tmp/content.txt";
    metadata.name = "content.txt";
    metadata.extension = ".txt";
    metadata.size = 25;
    metadata.modified_time = 100;

    int file_id = repository.saveFileMetadata(metadata);
    ASSERT_GT(file_id, 0);

    ASSERT_TRUE(repository.saveFileText(file_id, "hello world from database"));

    std::optional<std::string> text = repository.findTextByPath("/tmp/content.txt");

    ASSERT_TRUE(text.has_value());
    EXPECT_EQ(text.value(), "hello world from database");
}

TEST(RepositoryTest, ReturnsEmptyOptionalWhenTextDoesNotExist) {
    Database database;
    ASSERT_TRUE(database.open(":memory:"));
    ASSERT_TRUE(database.initializeSchema());

    Repository repository(database.connection());

    std::optional<std::string> text = repository.findTextByPath("/tmp/missing.txt");

    EXPECT_FALSE(text.has_value());
}
