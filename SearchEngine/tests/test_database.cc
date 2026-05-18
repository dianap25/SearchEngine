// Authors: Alesia Filinkova, Diana Pelin


#include <gtest/gtest.h>

#include "Database.h"

#include <cstdio>
#include <sqlite3.h>
#include <string>

namespace {

bool tableExists(sqlite3* db, const std::string& table_name) {
    const char* sql =
        "SELECT name FROM sqlite_master WHERE type='table' AND name=?;";

    sqlite3_stmt* statement = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &statement, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(statement, 1, table_name.c_str(), -1, SQLITE_TRANSIENT);

    bool exists = sqlite3_step(statement) == SQLITE_ROW;

    sqlite3_finalize(statement);
    return exists;
}

bool indexExists(sqlite3* db, const std::string& index_name) {
    const char* sql =
        "SELECT name FROM sqlite_master WHERE type='index' AND name=?;";

    sqlite3_stmt* statement = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &statement, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(statement, 1, index_name.c_str(), -1, SQLITE_TRANSIENT);

    bool exists = sqlite3_step(statement) == SQLITE_ROW;

    sqlite3_finalize(statement);
    return exists;
}

bool columnExists(sqlite3* db, const std::string& table_name, const std::string& column_name) {
    std::string sql = "PRAGMA table_info(" + table_name + ");";
    sqlite3_stmt* statement = nullptr;
    
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &statement, nullptr) != SQLITE_OK) {
        return false;
    }
    
    bool exists = false;
    while (sqlite3_step(statement) == SQLITE_ROW) {
        const unsigned char* name = sqlite3_column_text(statement, 1);
        if (name != nullptr && column_name == reinterpret_cast<const char*>(name)) {
            exists = true;
            break;
        }
    }
    
    sqlite3_finalize(statement);
    return exists;
}

int getTableRowCount(sqlite3* db, const std::string& table_name) {
    std::string sql = "SELECT COUNT(*) FROM " + table_name + ";";
    sqlite3_stmt* statement = nullptr;
    
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &statement, nullptr) != SQLITE_OK) {
        return -1;
    }
    
    int count = 0;
    if (sqlite3_step(statement) == SQLITE_ROW) {
        count = sqlite3_column_int(statement, 0);
    }
    
    sqlite3_finalize(statement);
    return count;
}

void insertTestFile(sqlite3* db, int id, const std::string& path, const std::string& name) {
    std::string sql = "INSERT INTO files (id, path, name, size, modified_time, indexed_at) VALUES (" +
                      std::to_string(id) + ", '" + path + "', '" + name + "', 100, 1234567890, 1234567890);";
    sqlite3_exec(db, sql.c_str(), nullptr, nullptr, nullptr);
}

} // namespace


TEST(DatabaseTest, OpensDatabaseFile) {
    const std::string db_path = "open_database_test.db";
    std::remove(db_path.c_str());

    {
        Database database;

        EXPECT_TRUE(database.open(db_path));
    }

    std::remove(db_path.c_str());
}

TEST(DatabaseTest, OpensDatabaseFileCreatesNewFile) {
    const std::string db_path = "new_database_test.db";
    std::remove(db_path.c_str());

    Database database;
    EXPECT_TRUE(database.open(db_path));
    
    // Sprawdź czy plik został utworzony
    FILE* file = fopen(db_path.c_str(), "r");
    EXPECT_NE(file, nullptr);
    if (file) fclose(file);
    
    std::remove(db_path.c_str());
}

TEST(DatabaseTest, OpensExistingDatabaseFile) {
    const std::string db_path = "existing_database_test.db";
    std::remove(db_path.c_str());
    
    {
        Database database;
        ASSERT_TRUE(database.open(db_path));
        ASSERT_TRUE(database.initializeSchema());
    }
    
    {
        Database database;
        EXPECT_TRUE(database.open(db_path));
    }
    
    std::remove(db_path.c_str());
}

TEST(DatabaseTest, FailsToOpenInvalidPath) {
    Database database;
    
    EXPECT_FALSE(database.open("/nonexistent/directory/database.db"));
}

TEST(DatabaseTest, CanOpenMultipleTimes) {
    const std::string db_path = "multiple_open_test.db";
    std::remove(db_path.c_str());
    
    Database database;
    
    EXPECT_TRUE(database.open(db_path));
    EXPECT_TRUE(database.open(db_path));  
    EXPECT_TRUE(database.open(db_path));  
    
    std::remove(db_path.c_str());
}


TEST(DatabaseTest, InitializesSchemaAfterOpen) {
    const std::string db_path = "initialize_schema_test.db";
    std::remove(db_path.c_str());

    {
        Database database;

        ASSERT_TRUE(database.open(db_path));
        EXPECT_TRUE(database.initializeSchema());
    }

    std::remove(db_path.c_str());
}

TEST(DatabaseTest, FailsToInitializeSchemaWithoutOpen) {
    Database database;

    EXPECT_FALSE(database.initializeSchema());
}

TEST(DatabaseTest, InitializeSchemaIsIdempotent) {
    const std::string db_path = "idempotent_schema_test.db";
    std::remove(db_path.c_str());

    {
        Database database;
        ASSERT_TRUE(database.open(db_path));
        
        EXPECT_TRUE(database.initializeSchema());
        EXPECT_TRUE(database.initializeSchema());
    }

    std::remove(db_path.c_str());
}

TEST(DatabaseTest, CreatesRequiredTablesAndIndexes) {
    const std::string db_path = "schema_test.db";
    std::remove(db_path.c_str());

    {
        Database database;

        ASSERT_TRUE(database.open(db_path));
        ASSERT_TRUE(database.initializeSchema());
    }

    sqlite3* raw_db = nullptr;
    ASSERT_EQ(sqlite3_open(db_path.c_str(), &raw_db), SQLITE_OK);

    EXPECT_TRUE(tableExists(raw_db, "files"));
    EXPECT_TRUE(tableExists(raw_db, "terms"));
    EXPECT_TRUE(tableExists(raw_db, "postings"));
    EXPECT_TRUE(tableExists(raw_db, "file_texts"));

    EXPECT_TRUE(indexExists(raw_db, "idx_files_path"));
    EXPECT_TRUE(indexExists(raw_db, "idx_files_name"));
    EXPECT_TRUE(indexExists(raw_db, "idx_terms_term"));
    EXPECT_TRUE(indexExists(raw_db, "idx_postings_term_file"));

    sqlite3_close(raw_db);
    std::remove(db_path.c_str());
}

TEST(DatabaseTest, CreatesAllRequiredColumnsInFilesTable) {
    const std::string db_path = "files_columns_test.db";
    std::remove(db_path.c_str());

    {
        Database database;
        ASSERT_TRUE(database.open(db_path));
        ASSERT_TRUE(database.initializeSchema());
    }

    sqlite3* raw_db = nullptr;
    ASSERT_EQ(sqlite3_open(db_path.c_str(), &raw_db), SQLITE_OK);

    EXPECT_TRUE(columnExists(raw_db, "files", "id"));
    EXPECT_TRUE(columnExists(raw_db, "files", "path"));
    EXPECT_TRUE(columnExists(raw_db, "files", "name"));
    EXPECT_TRUE(columnExists(raw_db, "files", "extension"));
    EXPECT_TRUE(columnExists(raw_db, "files", "size"));
    EXPECT_TRUE(columnExists(raw_db, "files", "modified_time"));
    EXPECT_TRUE(columnExists(raw_db, "files", "indexed_at"));
    EXPECT_TRUE(columnExists(raw_db, "files", "content_hash"));

    sqlite3_close(raw_db);
    std::remove(db_path.c_str());
}

TEST(DatabaseTest, CreatesAllRequiredColumnsInTermsTable) {
    const std::string db_path = "terms_columns_test.db";
    std::remove(db_path.c_str());

    {
        Database database;
        ASSERT_TRUE(database.open(db_path));
        ASSERT_TRUE(database.initializeSchema());
    }

    sqlite3* raw_db = nullptr;
    ASSERT_EQ(sqlite3_open(db_path.c_str(), &raw_db), SQLITE_OK);

    EXPECT_TRUE(columnExists(raw_db, "terms", "id"));
    EXPECT_TRUE(columnExists(raw_db, "terms", "term"));

    sqlite3_close(raw_db);
    std::remove(db_path.c_str());
}

TEST(DatabaseTest, CreatesAllRequiredColumnsInPostingsTable) {
    const std::string db_path = "postings_columns_test.db";
    std::remove(db_path.c_str());

    {
        Database database;
        ASSERT_TRUE(database.open(db_path));
        ASSERT_TRUE(database.initializeSchema());
    }

    sqlite3* raw_db = nullptr;
    ASSERT_EQ(sqlite3_open(db_path.c_str(), &raw_db), SQLITE_OK);

    EXPECT_TRUE(columnExists(raw_db, "postings", "term_id"));
    EXPECT_TRUE(columnExists(raw_db, "postings", "file_id"));
    EXPECT_TRUE(columnExists(raw_db, "postings", "position"));

    sqlite3_close(raw_db);
    std::remove(db_path.c_str());
}

TEST(DatabaseTest, CreatesAllRequiredColumnsInFileTextsTable) {
    const std::string db_path = "file_texts_columns_test.db";
    std::remove(db_path.c_str());

    {
        Database database;
        ASSERT_TRUE(database.open(db_path));
        ASSERT_TRUE(database.initializeSchema());
    }

    sqlite3* raw_db = nullptr;
    ASSERT_EQ(sqlite3_open(db_path.c_str(), &raw_db), SQLITE_OK);

    EXPECT_TRUE(columnExists(raw_db, "file_texts", "file_id"));
    EXPECT_TRUE(columnExists(raw_db, "file_texts", "content"));

    sqlite3_close(raw_db);
    std::remove(db_path.c_str());
}


TEST(DatabaseTest, EnablesForeignKeySupport) {
    const std::string db_path = "foreign_keys_test.db";
    std::remove(db_path.c_str());

    Database database;
    ASSERT_TRUE(database.open(db_path));
    ASSERT_TRUE(database.initializeSchema());
    
    // Użyj tego samego połączenia - Database::connection()
    sqlite3* raw_db = database.connection();
    ASSERT_NE(raw_db, nullptr);
    
    sqlite3_stmt* statement = nullptr;
    ASSERT_EQ(sqlite3_prepare_v2(raw_db, "PRAGMA foreign_keys;", -1, &statement, nullptr), SQLITE_OK);
    
    int fk_enabled = 0;
    if (sqlite3_step(statement) == SQLITE_ROW) {
        fk_enabled = sqlite3_column_int(statement, 0);
    }
    
    EXPECT_EQ(fk_enabled, 1);
    
    sqlite3_finalize(statement);
    std::remove(db_path.c_str());
}


TEST(DatabaseTest, ConnectionReturnsNullptrWhenNotOpen) {
    Database database;
    
    EXPECT_EQ(database.connection(), nullptr);
}

TEST(DatabaseTest, ConnectionReturnsNonNullptrWhenOpen) {
    const std::string db_path = "connection_test.db";
    std::remove(db_path.c_str());

    Database database;
    ASSERT_TRUE(database.open(db_path));
    
    EXPECT_NE(database.connection(), nullptr);
    
    std::remove(db_path.c_str());
}

TEST(DatabaseTest, ConnectionReturnsSameHandleAfterSchemaInitialization) {
    const std::string db_path = "connection_handle_test.db";
    std::remove(db_path.c_str());

    Database database;
    ASSERT_TRUE(database.open(db_path));
    
    sqlite3* conn1 = database.connection();
    ASSERT_TRUE(database.initializeSchema());
    sqlite3* conn2 = database.connection();
    
    EXPECT_EQ(conn1, conn2);
    
    std::remove(db_path.c_str());
}


TEST(DatabaseTest, DatabaseClosesAutomaticallyOnDestruction) {
    const std::string db_path = "auto_close_test.db";
    std::remove(db_path.c_str());

    {
        Database database;
        ASSERT_TRUE(database.open(db_path));
        ASSERT_TRUE(database.initializeSchema());
        
        EXPECT_NE(database.connection(), nullptr);
    }
    
    sqlite3* raw_db = nullptr;
    EXPECT_EQ(sqlite3_open(db_path.c_str(), &raw_db), SQLITE_OK);
    
    if (raw_db != nullptr) {
        sqlite3_close(raw_db);
    }
    
    std::remove(db_path.c_str());
}


TEST(DatabaseTest, MigrationAddsContentHashColumn) {
    const std::string db_path = "migration_test.db";
    std::remove(db_path.c_str());

    sqlite3* raw_db = nullptr;
    ASSERT_EQ(sqlite3_open(db_path.c_str(), &raw_db), SQLITE_OK);
    
    const char* create_table_sql = R"(
        CREATE TABLE files (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            path TEXT NOT NULL UNIQUE,
            name TEXT NOT NULL,
            extension TEXT,
            size INTEGER NOT NULL,
            modified_time INTEGER NOT NULL,
            indexed_at INTEGER NOT NULL
        );
    )";
    ASSERT_EQ(sqlite3_exec(raw_db, create_table_sql, nullptr, nullptr, nullptr), SQLITE_OK);
    
    sqlite3_close(raw_db);
    
    {
        Database database;
        ASSERT_TRUE(database.open(db_path));
        ASSERT_TRUE(database.initializeSchema());
    }
    
    ASSERT_EQ(sqlite3_open(db_path.c_str(), &raw_db), SQLITE_OK);
    EXPECT_TRUE(columnExists(raw_db, "files", "content_hash"));
    
    sqlite3_close(raw_db);
    std::remove(db_path.c_str());
}


TEST(DatabaseTest, DatabaseCanBeOpenedAndClosedMultipleTimes) {
    const std::string db_path = "multiple_cycles_test.db";
    std::remove(db_path.c_str());
    
    for (int i = 0; i < 3; ++i) {
        Database database;
        EXPECT_TRUE(database.open(db_path));
        EXPECT_TRUE(database.initializeSchema());
    }
    
    std::remove(db_path.c_str());
}


TEST(DatabaseTest, CanInsertAndQueryDataAfterSchemaInitialization) {
    const std::string db_path = "integration_test.db";
    std::remove(db_path.c_str());

    {
        Database database;
        ASSERT_TRUE(database.open(db_path));
        ASSERT_TRUE(database.initializeSchema());
        
        sqlite3* db = database.connection();
        ASSERT_NE(db, nullptr);
        
        const char* insert_sql = R"(
            INSERT INTO files (path, name, size, modified_time, indexed_at) 
            VALUES ('/test/file.txt', 'file.txt', 1024, 1234567890, 1234567890);
        )";
        EXPECT_EQ(sqlite3_exec(db, insert_sql, nullptr, nullptr, nullptr), SQLITE_OK);
        
        sqlite3_stmt* stmt = nullptr;
        const char* select_sql = "SELECT COUNT(*) FROM files WHERE name = 'file.txt';";
        ASSERT_EQ(sqlite3_prepare_v2(db, select_sql, -1, &stmt, nullptr), SQLITE_OK);
        
        int count = 0;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }
        
        EXPECT_EQ(count, 1);
        
        sqlite3_finalize(stmt);
    }
    
    std::remove(db_path.c_str());
}
