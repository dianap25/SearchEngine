//Alesia Filinkova
//Diana Pelin

#include <gtest/gtest.h>

#include "Database.h"
#include "IndexService.h"
#include "Repository.h"

#include <filesystem>
#include <fstream>

TEST(IndexServiceTest, IndexesDirectoryWithTextFile) {
    const std::filesystem::path testDir = "index_service_test_data";
    std::filesystem::create_directories(testDir);

    const std::filesystem::path filePath = testDir / "example.txt";

    {
        std::ofstream file(filePath);
        file << "hello world hello";
    }

    Database database;
    ASSERT_TRUE(database.open(":memory:"));
    ASSERT_TRUE(database.initializeSchema());

    IndexService indexService(database);
    IndexSummary summary = indexService.indexDirectory(testDir.string());

    EXPECT_EQ(summary.scannedFiles, 1);
    EXPECT_EQ(summary.indexedFiles, 1);
    EXPECT_EQ(summary.skippedFiles, 0);
    EXPECT_EQ(summary.failedFiles, 0);

    Repository repository(database.connection());
    std::vector<SearchResult> results = repository.searchByContent("hello");

    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].occurrences, 2);

    std::filesystem::remove_all(testDir);
}