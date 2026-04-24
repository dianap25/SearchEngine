#include <gtest/gtest.h>
#include "Database.h"

TEST(DatabaseTest, OpensDatabaseFile) {
    Database database;
    EXPECT_TRUE(database.open("test.db"));
}

TEST(DatabaseTest, InitializesSchemaAfterOpen) {
    Database database;
    ASSERT_TRUE(database.open("test.db"));
    EXPECT_TRUE(database.initializeSchema());
}

TEST(DatabaseTest, FailsToInitializeSchemaWithoutOpen) {
    Database database;
    EXPECT_FALSE(database.initializeSchema());
}