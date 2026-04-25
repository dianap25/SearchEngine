//Alesia Filinkova
//Diana Pelin

#include <gtest/gtest.h>

#include "Database.h"

#include <cstdio>
#include <sqlite3.h>
#include <string>

namespace {

bool tableExists(sqlite3* db, const std::string& tableName) {
    const char* sql =
        "SELECT name FROM sqlite_master WHERE type='table' AND name=?;";

    sqlite3_stmt* statement = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &statement, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(statement, 1, tableName.c_str(), -1, SQLITE_TRANSIENT);

    bool exists = sqlite3_step(statement) == SQLITE_ROW;

    sqlite3_finalize(statement);
    return exists;
}

bool indexExists(sqlite3* db, const std::string& indexName) {
    const char* sql =
        "SELECT name FROM sqlite_master WHERE type='index' AND name=?;";

    sqlite3_stmt* statement = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &statement, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(statement, 1, indexName.c_str(), -1, SQLITE_TRANSIENT);

    bool exists = sqlite3_step(statement) == SQLITE_ROW;

    sqlite3_finalize(statement);
    return exists;
}

} // namespace

TEST(DatabaseTest, OpensDatabaseFile) {
    const std::string dbPath = "open_database_test.db";
    std::remove(dbPath.c_str());

    {
        Database database;

        EXPECT_TRUE(database.open(dbPath));
    }

    std::remove(dbPath.c_str());
}

TEST(DatabaseTest, InitializesSchemaAfterOpen) {
    const std::string dbPath = "initialize_schema_test.db";
    std::remove(dbPath.c_str());

    {
        Database database;

        ASSERT_TRUE(database.open(dbPath));
        EXPECT_TRUE(database.initializeSchema());
    }

    std::remove(dbPath.c_str());
}

TEST(DatabaseTest, FailsToInitializeSchemaWithoutOpen) {
    Database database;

    EXPECT_FALSE(database.initializeSchema());
}

TEST(DatabaseTest, CreatesRequiredTablesAndIndexes) {
    const std::string dbPath = "schema_test.db";
    std::remove(dbPath.c_str());

    {
        Database database;

        ASSERT_TRUE(database.open(dbPath));
        ASSERT_TRUE(database.initializeSchema());
    }

    sqlite3* rawDb = nullptr;
    ASSERT_EQ(sqlite3_open(dbPath.c_str(), &rawDb), SQLITE_OK);

    EXPECT_TRUE(tableExists(rawDb, "files"));
    EXPECT_TRUE(tableExists(rawDb, "terms"));
    EXPECT_TRUE(tableExists(rawDb, "postings"));
    EXPECT_TRUE(tableExists(rawDb, "file_texts"));

    EXPECT_TRUE(indexExists(rawDb, "idx_files_path"));
    EXPECT_TRUE(indexExists(rawDb, "idx_files_name"));
    EXPECT_TRUE(indexExists(rawDb, "idx_terms_term"));
    EXPECT_TRUE(indexExists(rawDb, "idx_postings_term_file"));

    sqlite3_close(rawDb);
    std::remove(dbPath.c_str());
}