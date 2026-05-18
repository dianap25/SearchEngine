// Authors: Alesia Filinkova, Diana Pelin


#include <gtest/gtest.h>
#include "Database.h"
#include "IndexService.h"
#include "RefreshEngine.h"
#include "Repository.h"
#include "Hasher.h"

#include <sqlite3.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <thread>

namespace fs = std::filesystem;

class RefreshEngineTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create unique temporary directory for each test
        scratch_dir = fs::temp_directory_path() /
                      ("refresh_engine_test_" + std::to_string(::getpid()));
        fs::create_directories(scratch_dir);

        ASSERT_TRUE(database.open(":memory:"));
        ASSERT_TRUE(database.initializeSchema());
        
        repository = std::make_unique<Repository>(database.connection());
    }

    void TearDown() override {
        repository.reset();
        std::error_code ec;
        fs::remove_all(scratch_dir, ec);
    }

    fs::path writeFile(const std::string& name, const std::string& contents) {
        fs::path path = scratch_dir / name;
        std::ofstream stream(path);
        stream << contents;
        return path;
    }

    void modifyFile(const fs::path& path, const std::string& new_contents) {
        std::ofstream stream(path);
        stream << new_contents;
    }

    int countFiles() {
        sqlite3* db = database.connection();
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM files;", -1, &stmt, nullptr) != SQLITE_OK) {
            return -1;
        }
        int count = 0;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
        return count;
    }

    int countPostings() {
        sqlite3* db = database.connection();
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM postings;", -1, &stmt, nullptr) != SQLITE_OK) {
            return -1;
        }
        int count = 0;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
        return count;
    }

    int countTerms() {
        sqlite3* db = database.connection();
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM terms;", -1, &stmt, nullptr) != SQLITE_OK) {
            return -1;
        }
        int count = 0;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
        return count;
    }

    std::optional<FileMetadata> getFileMetadata(const std::string& path) {
        return repository->findByPath(path);
    }

    std::optional<std::string> getFileContent(const std::string& path) {
        return repository->findTextByPath(path);
    }

    std::int64_t getIndexedAt(const std::string& path) {
        sqlite3* db = database.connection();
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, "SELECT indexed_at FROM files WHERE path = ?;", -1, &stmt, nullptr) != SQLITE_OK) {
            return -1;
        }
        sqlite3_bind_text(stmt, 1, path.c_str(), -1, SQLITE_TRANSIENT);
        std::int64_t value = -1;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            value = sqlite3_column_int64(stmt, 0);
        }
        sqlite3_finalize(stmt);
        return value;
    }

    fs::path scratch_dir;
    Database database;
    std::unique_ptr<Repository> repository;
};

TEST_F(RefreshEngineTest, FirstRefreshIndexesAllFiles) {
    writeFile("a.txt", "alpha beta gamma");
    writeFile("b.txt", "delta epsilon");
    writeFile("c.txt", "zeta eta theta");

    RefreshEngine engine;
    engine.refresh(scratch_dir.string(), database);

    EXPECT_EQ(countFiles(), 3);
    EXPECT_GT(countPostings(), 0);
    EXPECT_GT(countTerms(), 0);
}

TEST_F(RefreshEngineTest, FirstRefreshWithSubdirectories) {
    fs::create_directories(scratch_dir / "subdir");
    writeFile("subdir/file1.txt", "content1");
    writeFile("subdir/file2.txt", "content2");
    writeFile("root.txt", "root content");

    RefreshEngine engine;
    engine.refresh(scratch_dir.string(), database);

    EXPECT_EQ(countFiles(), 3);
}

TEST_F(RefreshEngineTest, FirstRefreshWithEmptyDirectory) {
    RefreshEngine engine;
    engine.refresh(scratch_dir.string(), database);

    EXPECT_EQ(countFiles(), 0);
    EXPECT_EQ(countPostings(), 0);
}

TEST_F(RefreshEngineTest, FirstRefreshWithUnsupportedFileType) {
    writeFile("a.txt", "supported");
    writeFile("b.xyz", "unsupported");
    writeFile("c", "no extension");

    RefreshEngine engine;
    engine.refresh(scratch_dir.string(), database);

    EXPECT_EQ(countFiles(), 2);
}

TEST_F(RefreshEngineTest, SecondRefreshIsNoOpWhenNothingChanged) {
    writeFile("a.txt", "unchanged content");
    writeFile("b.txt", "another content");

    RefreshEngine engine;
    engine.refresh(scratch_dir.string(), database);

    int files_before = countFiles();
    int postings_before = countPostings();
    
    std::int64_t indexed_at_before = getIndexedAt((scratch_dir / "a.txt").string());
    std::string hash_before = getFileMetadata((scratch_dir / "a.txt").string())->content_hash;

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    engine.refresh(scratch_dir.string(), database);

    EXPECT_EQ(countFiles(), files_before);
    EXPECT_EQ(countPostings(), postings_before);
    EXPECT_EQ(getIndexedAt((scratch_dir / "a.txt").string()), indexed_at_before);
    
    auto metadata = getFileMetadata((scratch_dir / "a.txt").string());
    ASSERT_TRUE(metadata.has_value());
    EXPECT_EQ(metadata->content_hash, hash_before);
}

TEST_F(RefreshEngineTest, ModifiedFileContentIsUpdated) {
    fs::path a = writeFile("a.txt", "original content");

    RefreshEngine engine;
    engine.refresh(scratch_dir.string(), database);

    // Verify original content
    auto content_before = getFileContent(a.string());
    ASSERT_TRUE(content_before.has_value());
    EXPECT_EQ(content_before.value(), "original content");

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    modifyFile(a, "modified content different");

    engine.refresh(scratch_dir.string(), database);

    auto content_after = getFileContent(a.string());
    ASSERT_TRUE(content_after.has_value());
    EXPECT_EQ(content_after.value(), "modified content different");
}

TEST_F(RefreshEngineTest, ModifiedFileWithSameHashIsSkipped) {
    fs::path a = writeFile("a.txt", "content");

    RefreshEngine engine;
    engine.refresh(scratch_dir.string(), database);

    std::int64_t indexed_at_before = getIndexedAt(a.string());

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    modifyFile(a, "content"); // Same content

    engine.refresh(scratch_dir.string(), database);

    EXPECT_EQ(getIndexedAt(a.string()), indexed_at_before);
}

TEST_F(RefreshEngineTest, MultipleFilesContentUpdated) {
    fs::path a = writeFile("a.txt", "aaa");
    fs::path b = writeFile("b.txt", "bbb");
    fs::path c = writeFile("c.txt", "ccc");

    RefreshEngine engine;
    engine.refresh(scratch_dir.string(), database);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    modifyFile(a, "aaa modified");
    modifyFile(c, "ccc modified");

    engine.refresh(scratch_dir.string(), database);

    auto content_a = getFileContent(a.string());
    auto content_b = getFileContent(b.string());
    auto content_c = getFileContent(c.string());
    
    ASSERT_TRUE(content_a.has_value());
    ASSERT_TRUE(content_b.has_value());
    ASSERT_TRUE(content_c.has_value());
    
    EXPECT_EQ(content_a.value(), "aaa modified");
    EXPECT_EQ(content_b.value(), "bbb");
    EXPECT_EQ(content_c.value(), "ccc modified");
}

TEST_F(RefreshEngineTest, NewFileIsAddedDuringRefresh) {
    writeFile("a.txt", "file a");

    RefreshEngine engine;
    engine.refresh(scratch_dir.string(), database);
    EXPECT_EQ(countFiles(), 1);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    writeFile("b.txt", "file b");
    writeFile("c.txt", "file c");

    engine.refresh(scratch_dir.string(), database);

    EXPECT_EQ(countFiles(), 3);
}

TEST_F(RefreshEngineTest, NewFileInSubdirectoryIsAdded) {
    RefreshEngine engine;
    engine.refresh(scratch_dir.string(), database);
    EXPECT_EQ(countFiles(), 0);

    fs::create_directories(scratch_dir / "new_subdir");
    writeFile("new_subdir/added.txt", "added content");

    engine.refresh(scratch_dir.string(), database);

    EXPECT_EQ(countFiles(), 1);
}

TEST_F(RefreshEngineTest, RemovedFileIsDeletedFromIndex) {
    fs::path a = writeFile("a.txt", "keep");
    fs::path b = writeFile("b.txt", "delete");

    RefreshEngine engine;
    engine.refresh(scratch_dir.string(), database);
    EXPECT_EQ(countFiles(), 2);

    fs::remove(b);

    engine.refresh(scratch_dir.string(), database);

    EXPECT_EQ(countFiles(), 1);
    EXPECT_TRUE(getFileMetadata(a.string()).has_value());
    EXPECT_FALSE(getFileMetadata(b.string()).has_value());
}

TEST_F(RefreshEngineTest, RemovedFileCleansUpPostings) {
    writeFile("a.txt", "unique content for deletion test");

    RefreshEngine engine;
    engine.refresh(scratch_dir.string(), database);

    int postings_before = countPostings();
    ASSERT_GT(postings_before, 0);

    fs::remove(scratch_dir / "a.txt");

    engine.refresh(scratch_dir.string(), database);

    EXPECT_EQ(countPostings(), 0);
}

TEST_F(RefreshEngineTest, RemovedDirectoryCleansUpAllFiles) {
    fs::create_directories(scratch_dir / "subdir");
    writeFile("subdir/file1.txt", "content1");
    writeFile("subdir/file2.txt", "content2");
    writeFile("root.txt", "root content");

    RefreshEngine engine;
    engine.refresh(scratch_dir.string(), database);
    EXPECT_EQ(countFiles(), 3);

    fs::remove_all(scratch_dir / "subdir");

    engine.refresh(scratch_dir.string(), database);

    EXPECT_EQ(countFiles(), 1);
    EXPECT_TRUE(getFileMetadata((scratch_dir / "root.txt").string()).has_value());
    EXPECT_FALSE(getFileMetadata((scratch_dir / "subdir/file1.txt").string()).has_value());
    EXPECT_FALSE(getFileMetadata((scratch_dir / "subdir/file2.txt").string()).has_value());
}

TEST_F(RefreshEngineTest, AddModifyDeleteInSameRefresh) {
    writeFile("keep.txt", "keep this file");
    writeFile("modify.txt", "original content");
    writeFile("delete.txt", "to be deleted");

    RefreshEngine engine;
    engine.refresh(scratch_dir.string(), database);
    
    std::string modify_hash_before = getFileMetadata((scratch_dir / "modify.txt").string())->content_hash;
    std::string keep_hash_before = getFileMetadata((scratch_dir / "keep.txt").string())->content_hash;
    auto modify_content_before = getFileContent((scratch_dir / "modify.txt").string());
    ASSERT_TRUE(modify_content_before.has_value());
    EXPECT_EQ(modify_content_before.value(), "original content");

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    writeFile("new.txt", "brand new file");
    modifyFile(scratch_dir / "modify.txt", "modified content");
    fs::remove(scratch_dir / "delete.txt");

    engine.refresh(scratch_dir.string(), database);

    EXPECT_EQ(countFiles(), 3); // keep.txt, modify.txt, new.txt
    EXPECT_FALSE(getFileMetadata((scratch_dir / "delete.txt").string()).has_value());
    
    EXPECT_TRUE(getFileMetadata((scratch_dir / "new.txt").string()).has_value());
    auto new_content = getFileContent((scratch_dir / "new.txt").string());
    ASSERT_TRUE(new_content.has_value());
    EXPECT_EQ(new_content.value(), "brand new file");
    
    auto modify_metadata = getFileMetadata((scratch_dir / "modify.txt").string());
    ASSERT_TRUE(modify_metadata.has_value());
    EXPECT_NE(modify_metadata->content_hash, modify_hash_before);
    
    auto modify_content_after = getFileContent((scratch_dir / "modify.txt").string());
    ASSERT_TRUE(modify_content_after.has_value());
    EXPECT_EQ(modify_content_after.value(), "modified content");
    
    auto keep_metadata = getFileMetadata((scratch_dir / "keep.txt").string());
    ASSERT_TRUE(keep_metadata.has_value());
    EXPECT_EQ(keep_metadata->content_hash, keep_hash_before);
    
    auto keep_content = getFileContent((scratch_dir / "keep.txt").string());
    ASSERT_TRUE(keep_content.has_value());
    EXPECT_EQ(keep_content.value(), "keep this file");
}

TEST_F(RefreshEngineTest, HandlesNonExistentRootPath) {
    fs::path non_existent = scratch_dir / "does_not_exist";

    RefreshEngine engine;
    engine.refresh(non_existent.string(), database);

    EXPECT_EQ(countFiles(), 0);
}

TEST_F(RefreshEngineTest, HandlesFileWithNoExtractor) {
    writeFile("a.xyz", "some content");

    RefreshEngine engine;
    engine.refresh(scratch_dir.string(), database);

    EXPECT_EQ(countFiles(), 0);
}

TEST_F(RefreshEngineTest, RefreshIsAtomicOnFailure) {
    writeFile("a.txt", "content a");
    writeFile("b.txt", "content b");

    // First refresh succeeds
    RefreshEngine engine;
    engine.refresh(scratch_dir.string(), database);
    
    int files_before = countFiles();
    EXPECT_EQ(files_before, 2);

    // Create a file that will cause extraction to fail
    writeFile("c.xyz", "unsupported content");

    // Refresh should still work (skip unsupported)
    engine.refresh(scratch_dir.string(), database);
    
    EXPECT_EQ(countFiles(), 2); // Still only 2 files
}

TEST_F(RefreshEngineTest, ContentHashIsStoredCorrectly) {
    std::string content = "test content for hashing";
    fs::path file = writeFile("hash_test.txt", content);

    RefreshEngine engine;
    engine.refresh(scratch_dir.string(), database);

    auto metadata = getFileMetadata(file.string());
    ASSERT_TRUE(metadata.has_value());
    
    std::string expected_hash = Hasher::sha256(content);
    EXPECT_EQ(metadata->content_hash, expected_hash);
}

TEST_F(RefreshEngineTest, ContentHashChangesWhenFileModified) {
    fs::path file = writeFile("hash_test.txt", "original");

    RefreshEngine engine;
    engine.refresh(scratch_dir.string(), database);

    auto hash_before = getFileMetadata(file.string())->content_hash;

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    modifyFile(file, "modified content");

    engine.refresh(scratch_dir.string(), database);

    auto hash_after = getFileMetadata(file.string())->content_hash;
    EXPECT_NE(hash_after, hash_before);
}

TEST_F(RefreshEngineTest, RefreshPreservesFileContent) {
    std::string content = "This is the original file content";
    fs::path file = writeFile("content_test.txt", content);

    RefreshEngine engine;
    engine.refresh(scratch_dir.string(), database);

    auto saved_content = getFileContent(file.string());
    ASSERT_TRUE(saved_content.has_value());
    EXPECT_EQ(saved_content.value(), content);
}

TEST_F(RefreshEngineTest, RefreshUpdatesFileContentWhenModified) {
    fs::path file = writeFile("content_test.txt", "original");

    RefreshEngine engine;
    engine.refresh(scratch_dir.string(), database);

    std::string modified = "This is the modified content";
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    modifyFile(file, modified);

    engine.refresh(scratch_dir.string(), database);

    auto saved_content = getFileContent(file.string());
    ASSERT_TRUE(saved_content.has_value());
    EXPECT_EQ(saved_content.value(), modified);
}

TEST_F(RefreshEngineTest, RefreshHandlesMultipleRefreshCycles) {
    writeFile("a.txt", "first version");
    RefreshEngine engine;
    engine.refresh(scratch_dir.string(), database);
    EXPECT_EQ(countFiles(), 1);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    writeFile("b.txt", "second version");
    engine.refresh(scratch_dir.string(), database);
    EXPECT_EQ(countFiles(), 2);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    modifyFile(scratch_dir / "a.txt", "modified version");
    engine.refresh(scratch_dir.string(), database);
    
    auto content = getFileContent((scratch_dir / "a.txt").string());
    ASSERT_TRUE(content.has_value());
    EXPECT_EQ(content.value(), "modified version");
    
    fs::remove(scratch_dir / "b.txt");
    engine.refresh(scratch_dir.string(), database);
    EXPECT_EQ(countFiles(), 1);
}

TEST_F(RefreshEngineTest, RefreshCreatesSchemaIfNotExists) {
    Database new_db;
    ASSERT_TRUE(new_db.open(":memory:"));
    
    RefreshEngine engine;
    engine.refresh(scratch_dir.string(), new_db);
    
    sqlite3* db = new_db.connection();
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, "SELECT name FROM sqlite_master WHERE type='table';", -1, &stmt, nullptr);
    ASSERT_EQ(rc, SQLITE_OK);
    
    bool has_files_table = false;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        if (strcmp(name, "files") == 0) {
            has_files_table = true;
        }
    }
    sqlite3_finalize(stmt);
    EXPECT_TRUE(has_files_table);
}