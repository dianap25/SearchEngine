// Autorzy: Alesia Filinkova, Diana Pelin
// Opis: Testy jednostkowe klasy ContextBuilder. Pokrywają obcięcie
// marginesu na początku i końcu tekstu, brak frazy, dopasowanie
// niewrażliwe na wielkość liter, puste argumenty oraz domyślny
// margines 40 znaków.

#include <gtest/gtest.h>

#include "ContextBuilder.h"

TEST(ContextBuilderTest, ReturnsFragmentAroundPhrase) {
    ContextBuilder builder(5);

    std::string text = "abcdefghij needle klmnopqrst";
    std::string context = builder.build(text, "needle");

    EXPECT_EQ(context, "ghij needle klmn");
}

TEST(ContextBuilderTest, ClampsStartAtBeginningOfText) {
    ContextBuilder builder(40);

    std::string text = "hello world from database";
    std::string context = builder.build(text, "hello");

    EXPECT_EQ(context, "hello world from database");
}

TEST(ContextBuilderTest, ClampsEndAtEndOfText) {
    ContextBuilder builder(40);

    std::string text = "alpha beta gamma";
    std::string context = builder.build(text, "gamma");

    EXPECT_EQ(context, "alpha beta gamma");
}

TEST(ContextBuilderTest, ReturnsEmptyWhenPhraseNotFound) {
    ContextBuilder builder(10);

    std::string context = builder.build("some content", "missing");

    EXPECT_EQ(context, "");
}

TEST(ContextBuilderTest, FindsPhraseCaseInsensitively) {
    ContextBuilder builder(3);

    std::string text = "xxx Hello yyy";
    std::string context = builder.build(text, "hello");

    EXPECT_EQ(context, "xx Hello yy");
}

TEST(ContextBuilderTest, ReturnsEmptyForEmptyInputs) {
    ContextBuilder builder;

    EXPECT_EQ(builder.build("", "phrase"), "");
    EXPECT_EQ(builder.build("text", ""), "");
}

TEST(ContextBuilderTest, UsesDefaultMarginOf40) {
    ContextBuilder builder;

    std::string prefix(50, 'a');
    std::string suffix(50, 'b');
    std::string text = prefix + "needle" + suffix;

    std::string context = builder.build(text, "needle");

    EXPECT_EQ(context.size(), 40 + 6 + 40);
}
