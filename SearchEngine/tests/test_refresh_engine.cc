// Autorzy: Alesia Filinkova, Diana Pelin
// Opis: Testy jednostkowe klasy RefreshEngine. Wszystkie scenariusze
// uruchamiają się na bazie SQLite w pamięci oraz tymczasowym
// katalogu na dysku, dzięki czemu logika refreshu jest sprawdzana
// end-to-end bez dotykania danych produkcyjnych.

#include <gtest/gtest.h>

#include "Database.h"
#include "IndexService.h"
#include "RefreshEngine.h"
#include "Repository.h"

#include <sqlite3.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <thread>

namespace {

namespace fs = std::filesystem;

class RefreshEngineFixture : public ::testing::Test {
protected:
    fs::path scratch_dir;
    Database database;

    void SetUp() override {
        scratch_dir = fs::temp_directory_path() /
                      ("refresh_engine_test_" + std::to_string(::getpid()));
        fs::create_directories(scratch_dir);

        ASSERT_TRUE(database.open(":memory:"));
        ASSERT_TRUE(database.initializeSchema());
    }

    void TearDown() override {
        std::error_code ec;
        fs::remove_all(scratch_dir, ec);
    }

    fs::path writeFile(const std::string& name, const std::string& contents) {
        fs::path path = scratch_dir / name;
        std::ofstream stream(path);
        stream << contents;
        return path;
    }

    int countPostings() {
        sqlite3* db = database.connection();
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(
                db, "SELECT COUNT(*) FROM postings;", -1, &stmt, nullptr) != SQLITE_OK) {
            return -1;
        }
        int count = -1;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
        return count;
    }

    int countFiles() {
        sqlite3* db = database.connection();
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(
                db, "SELECT COUNT(*) FROM files;", -1, &stmt, nullptr) != SQLITE_OK) {
            return -1;
        }
        int count = -1;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
        return count;
    }

    std::int64_t indexedAtFor(const std::string& path) {
        sqlite3* db = database.connection();
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(
                db,
                "SELECT indexed_at FROM files WHERE path = ?;",
                -1,
                &stmt,
                nullptr) != SQLITE_OK) {
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
};

} // namespace

TEST_F(RefreshEngineFixture, FirstRefreshIndexesEverything) {
    writeFile("a.txt", "alpha alpha beta");
    writeFile("b.txt", "gamma");

    RefreshEngine engine;
    engine.refresh(scratch_dir.string(), database);

    EXPECT_EQ(countFiles(), 2);
    EXPECT_GT(countPostings(), 0);
}

TEST_F(RefreshEngineFixture, SecondRefreshIsNoOpWhenNothingChanged) {
    writeFile("a.txt", "alpha alpha beta");

    IndexService index_service(database);
    index_service.indexDirectory(scratch_dir.string());

    int postings_before = countPostings();
    fs::path a = scratch_dir / "a.txt";
    std::int64_t indexed_at_before = indexedAtFor(a.string());
    ASSERT_GT(indexed_at_before, 0);

    // Przesuń zegar, żeby ewentualny re-index podniósł indexed_at i
    // żebyśmy mogli to wykryć.
    std::this_thread::sleep_for(std::chrono::seconds(2));

    RefreshEngine engine;
    engine.refresh(scratch_dir.string(), database);

    EXPECT_EQ(countPostings(), postings_before);
    EXPECT_EQ(indexedAtFor(a.string()), indexed_at_before);
}

TEST_F(RefreshEngineFixture, ModifiedFileIsReIndexedTargetly) {
    writeFile("a.txt", "alpha");
    writeFile("b.txt", "beta");

    IndexService index_service(database);
    index_service.indexDirectory(scratch_dir.string());

    fs::path a = scratch_dir / "a.txt";
    fs::path b = scratch_dir / "b.txt";
    std::int64_t a_before = indexedAtFor(a.string());
    std::int64_t b_before = indexedAtFor(b.string());

    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Modyfikuj tylko a.txt.
    {
        std::ofstream stream(a);
        stream << "alpha gamma";
    }

    RefreshEngine engine;
    engine.refresh(scratch_dir.string(), database);

    EXPECT_GT(indexedAtFor(a.string()), a_before);
    EXPECT_EQ(indexedAtFor(b.string()), b_before);
}

TEST_F(RefreshEngineFixture, RemovedFileDisappearsFromIndex) {
    writeFile("keep.txt", "stays");
    fs::path drop = writeFile("drop.txt", "leaves");

    IndexService index_service(database);
    index_service.indexDirectory(scratch_dir.string());

    EXPECT_EQ(countFiles(), 2);

    fs::remove(drop);

    RefreshEngine engine;
    engine.refresh(scratch_dir.string(), database);

    EXPECT_EQ(countFiles(), 1);

    Repository repository(database.connection());
    EXPECT_FALSE(repository.findByPath(drop.string()).has_value());
    EXPECT_TRUE(repository.findByPath((scratch_dir / "keep.txt").string()).has_value());
}
