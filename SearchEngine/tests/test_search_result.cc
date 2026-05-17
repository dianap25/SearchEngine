// Authors: Alesia Filinkova, Diana Pelin

#include <gtest/gtest.h>
#include "SearchResult.h"

class SearchResultTest : public ::testing::Test {
protected:
    void SetUp() override {
        result.path = "/tmp/example.txt";
        result.name = "example.txt";
        result.occurrences = 5;
        result.context = "This is a sample context with the search term.";
    }
    
    SearchResult result;
};

TEST_F(SearchResultTest, HasPathField) {
    EXPECT_FALSE(result.path.empty());
    EXPECT_EQ(result.path, "/tmp/example.txt");
}

TEST_F(SearchResultTest, HasNameField) {
    EXPECT_FALSE(result.name.empty());
    EXPECT_EQ(result.name, "example.txt");
}

TEST_F(SearchResultTest, HasOccurrencesField) {
    EXPECT_GE(result.occurrences, 0);
    EXPECT_EQ(result.occurrences, 5);
}

TEST_F(SearchResultTest, HasContextField) {
    EXPECT_FALSE(result.context.empty());
    EXPECT_EQ(result.context, "This is a sample context with the search term.");
}

TEST_F(SearchResultTest, DefaultOccurrencesIsZero) {
    SearchResult default_result;
    EXPECT_EQ(default_result.occurrences, 0);
}

TEST_F(SearchResultTest, DefaultPathIsEmpty) {
    SearchResult default_result;
    EXPECT_TRUE(default_result.path.empty());
}

TEST_F(SearchResultTest, DefaultNameIsEmpty) {
    SearchResult default_result;
    EXPECT_TRUE(default_result.name.empty());
}

TEST_F(SearchResultTest, DefaultContextIsEmpty) {
    SearchResult default_result;
    EXPECT_TRUE(default_result.context.empty());
}

TEST_F(SearchResultTest, CanCopySearchResult) {
    SearchResult copy = result;
    
    EXPECT_EQ(copy.path, result.path);
    EXPECT_EQ(copy.name, result.name);
    EXPECT_EQ(copy.occurrences, result.occurrences);
    EXPECT_EQ(copy.context, result.context);
}

TEST_F(SearchResultTest, CanAssignSearchResult) {
    SearchResult assigned;
    assigned = result;
    
    EXPECT_EQ(assigned.path, result.path);
    EXPECT_EQ(assigned.name, result.name);
    EXPECT_EQ(assigned.occurrences, result.occurrences);
    EXPECT_EQ(assigned.context, result.context);
}

TEST_F(SearchResultTest, ResultsWithSameFieldsAreEqual) {
    SearchResult other;
    other.path = "/tmp/example.txt";
    other.name = "example.txt";
    other.occurrences = 5;
    other.context = "This is a sample context with the search term.";
    
    EXPECT_EQ(result.path, other.path);
    EXPECT_EQ(result.name, other.name);
    EXPECT_EQ(result.occurrences, other.occurrences);
    EXPECT_EQ(result.context, other.context);
}

TEST_F(SearchResultTest, ResultsWithDifferentContextsAreDifferent) {
    SearchResult other = result;
    other.context = "Different context";
    
    EXPECT_NE(result.context, other.context);
}