// Authors: Alesia Filinkova, Diana Pelin


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

    // Domyślny margines = 40, długość "needle" = 6
    EXPECT_EQ(context.size(), 40 + 6 + 40);
}

TEST(ContextBuilderTest, MarginLargerThanTextReturnsWholeText) {
    ContextBuilder builder(100);

    std::string text = "short text";
    std::string context = builder.build(text, "short");

    EXPECT_EQ(context, "short text");
}

TEST(ContextBuilderTest, PhraseAtBeginningWithSmallMargin) {
    ContextBuilder builder(3);

    std::string text = "needle at the beginning";
    std::string context = builder.build(text, "needle");

    // Oczekiwany wynik: "needle at " (6 znaków "needle" + 3 marginesu = 9 znaków)
    EXPECT_EQ(context, "needle at");
}

TEST(ContextBuilderTest, PhraseAtEndWithSmallMargin) {
    ContextBuilder builder(3);

    std::string text = "the end is needle";
    std::string context = builder.build(text, "needle");

    EXPECT_EQ(context, "is needle");
}

TEST(ContextBuilderTest, PhraseExactlyAtMarginBoundary) {
    ContextBuilder builder(5);

    std::string text = "12345needle67890";
    std::string context = builder.build(text, "needle");

    EXPECT_EQ(context, "12345needle67890");
}

TEST(ContextBuilderTest, ReturnsFirstOccurrence) {
    ContextBuilder builder(3);

    std::string text = "aaa needle bbb needle ccc";
    std::string context = builder.build(text, "needle");

    // Pierwsze wystąpienie "needle" jest na pozycji 4
    // Margines = 3, więc start = 4-3 = 1, end = 4+6+3 = 13
    // fragment od index 1, długość 12: "aa needle b"
    EXPECT_EQ(context, "aa needle bb");
}

TEST(ContextBuilderTest, FindsPhraseWithMixedCase) {
    ContextBuilder builder(5);

    std::string text = "find ThE wOrD here";
    std::string context = builder.build(text, "the word");

    EXPECT_EQ(context, "find ThE wOrD here");
}

TEST(ContextBuilderTest, SingleCharacterPhrase) {
    ContextBuilder builder(3);

    std::string text = "abcdefghij";
    std::string context = builder.build(text, "d");

    // "d" na pozycji 3, start = 3-3 = 0, end = 3+1+3 = 7
    // fragment od 0, długość 7: "abcdefg"
    EXPECT_EQ(context, "abcdefg");
}

TEST(ContextBuilderTest, ZeroMargin) {
    ContextBuilder builder(0);

    std::string text = "prefix needle suffix";
    std::string context = builder.build(text, "needle");

    EXPECT_EQ(context, "needle");
}

TEST(ContextBuilderTest, PhraseLongerThanText) {
    ContextBuilder builder(10);

    std::string text = "short";
    std::string context = builder.build(text, "this phrase is much longer than the text");

    EXPECT_EQ(context, "");
}

TEST(ContextBuilderTest, TextContainsOnlyPhrase) {
    ContextBuilder builder(5);

    std::string text = "needle";
    std::string context = builder.build(text, "needle");

    EXPECT_EQ(context, "needle");
}

TEST(ContextBuilderTest, MarginExceedsBothSides) {
    ContextBuilder builder(10);

    std::string text = "needle";
    std::string context = builder.build(text, "needle");

    EXPECT_EQ(context, "needle");
}

TEST(ContextBuilderTest, PreservesOriginalCaseInResult) {
    ContextBuilder builder(3);

    std::string text = "XXX HELLO YYY";
    std::string context = builder.build(text, "hello");

    EXPECT_EQ(context, "XX HELLO YY");
}

TEST(ContextBuilderTest, PhraseWithWhitespaceOnly) {
    ContextBuilder builder(5);

    std::string text = "a    b";
    std::string context = builder.build(text, "    ");

    EXPECT_EQ(context, "a    b");
}

TEST(ContextBuilderTest, VeryLargeMargin) {
    ContextBuilder builder(1000000);

    std::string text = "small text with needle inside";
    std::string context = builder.build(text, "needle");

    EXPECT_EQ(context, text);
}

TEST(ContextBuilderTest, PhraseNotFoundSimilarWords) {
    ContextBuilder builder(10);

    std::string text = "needful needlework needs needle";
    std::string context = builder.build(text, "needle");

    EXPECT_FALSE(context.empty());

    // Pierwsze wystąpienie "needle" jest w środku słowa "needlework"
    EXPECT_EQ(context, "needful needlework needs");
}

TEST(ContextBuilderTest, MarginMuchLargerThanTextFromBothSides) {
    ContextBuilder builder(100);

    std::string text = "abc needle xyz";
    std::string context = builder.build(text, "needle");

    EXPECT_EQ(context, "abc needle xyz");
}

TEST(ContextBuilderTest, DefaultMarginWithVariousTexts) {
    ContextBuilder builder;

    std::string text(50, 'x');
    text += "target";
    text += std::string(50, 'y');

    std::string context = builder.build(text, "target");

    // Domyślny margines = 40, więc całkowita długość = 40 + 6 + 40 = 86
    EXPECT_EQ(context.size(), 86);
    EXPECT_EQ(context.substr(40, 6), "target");
}
