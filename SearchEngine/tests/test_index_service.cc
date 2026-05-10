// Authors: Alesia Filinkova, Diana Pelin
// Description: Unit test for IndexService. Builds a small directory
// on the filesystem, runs the full index pipeline against it and
// asserts both the IndexSummary counters and the resulting search
// hit.

#include <gtest/gtest.h>

#include "Database.h"
#include "IndexService.h"
#include "Repository.h"

#include <filesystem>
#include <fstream>

TEST(IndexServiceTest, IndexesDirectoryWithTextFile) {
    const std::filesystem::path test_dir = "index_service_test_data";
    std::filesystem::create_directories(test_dir);

    const std::filesystem::path file_path = test_dir / "example.txt";

    {
        std::ofstream file(file_path);
        file << "hello world hello";
    }

    Database database;
    ASSERT_TRUE(database.open(":memory:"));
    ASSERT_TRUE(database.initializeSchema());

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

    std::filesystem::remove_all(test_dir);
}
