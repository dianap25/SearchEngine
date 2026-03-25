#include <gtest/gtest.h>
#include "Database.h"

TEST(DatabaseTest, OpensDatabaseFile) {
    Database database;
    EXPECT_TRUE(database.open("test.db"));
}