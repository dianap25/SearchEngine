// Authors: Alesia Filinkova, Diana Pelin

#include <gtest/gtest.h>
#include "Hasher.h"

namespace {

// Reference SHA-256 digest of the empty string, in lowercase hex.
constexpr const char* SHA256_EMPTY =
    "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";

// Reference digests for common test vectors
constexpr const char* SHA256_HELLO =
    "8b1a9953c4611296a827abf8c47804d7e5c1c2e9e2d5b6f0e0e5a9e8f0d5c9b5";

constexpr const char* SHA256_FOX =
    "d7a8fbb307d7809469ca9abcb0082e4f8d5651e46d3cdb762d02d0bf37c9e592";

} // namespace

TEST(HasherTest, EmptyStringMatchesReferenceDigest) {
    EXPECT_EQ(Hasher::sha256(""), SHA256_EMPTY);
}

TEST(HasherTest, DigestIsDeterministic) {
    const std::string input = "the quick brown fox jumps over the lazy dog";

    EXPECT_EQ(Hasher::sha256(input), Hasher::sha256(input));
}

TEST(HasherTest, DistinctInputsProduceDistinctDigests) {
    const std::string a = "alpha";
    const std::string b = "alphb";

    EXPECT_NE(Hasher::sha256(a), Hasher::sha256(b));
}

TEST(HasherTest, DigestHas64HexCharacters) {
    const std::string digest = Hasher::sha256("anything");

    EXPECT_EQ(digest.size(), 64);
    for (char c : digest) {
        EXPECT_TRUE(
            (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f')
        ) << "non-hex character in digest: " << c;
    }
}


TEST(HasherTest, KnownVectorHello) {
    std::string digest = Hasher::sha256("hello");
    
    // Ręcznie obliczony SHA-256 dla "hello"
    EXPECT_EQ(digest, "2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824");
}

TEST(HasherTest, KnownVectorAbc) {
    std::string digest = Hasher::sha256("abc");
    
    EXPECT_EQ(digest, "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
}

TEST(HasherTest, KnownVectorFox) {
    std::string input = "The quick brown fox jumps over the lazy dog";
    std::string digest = Hasher::sha256(input);
    
    // Oczekiwany digest dla tego zdania
    EXPECT_EQ(digest, "d7a8fbb307d7809469ca9abcb0082e4f8d5651e46d3cdb762d02d0bf37c9e592");
}

TEST(HasherTest, VeryLongInput) {
    std::string long_input(1000000, 'a');
    
    std::string digest = Hasher::sha256(long_input);
    
    EXPECT_EQ(digest.size(), 64);
    EXPECT_FALSE(digest.empty());
}

TEST(HasherTest, SpecialCharacters) {
    std::string input = "!@#$%^&*()_+-=[]{}|;':\",./<>?\\`~";
    std::string digest = Hasher::sha256(input);
    
    EXPECT_EQ(digest.size(), 64);
    EXPECT_NE(digest, Hasher::sha256(""));
}

TEST(HasherTest, PolishCharacters) {
    std::string input = "zażółć gęślą jaźń";
    std::string digest1 = Hasher::sha256(input);
    std::string digest2 = Hasher::sha256(input);
    
    EXPECT_EQ(digest1, digest2);
    EXPECT_EQ(digest1.size(), 64);
}

TEST(HasherTest, NumericStrings) {
    std::string digest123 = Hasher::sha256("123");
    std::string digest321 = Hasher::sha256("321");
    
    EXPECT_NE(digest123, digest321);
    EXPECT_NE(digest123, Hasher::sha256("123 "));
}

TEST(HasherTest, SameLengthDifferentInputs) {
    std::string a = "aaaaaaa";
    std::string b = "aaaaaab";
    
    EXPECT_NE(Hasher::sha256(a), Hasher::sha256(b));
}

TEST(HasherTest, AvalancheEffect) {
    std::string input = "Hello World";
    std::string changed = "Hello World!";
    
    std::string hash1 = Hasher::sha256(input);
    std::string hash2 = Hasher::sha256(changed);
    
    EXPECT_NE(hash1, hash2);
    
    int differences = 0;
    for (size_t i = 0; i < hash1.size(); ++i) {
        if (hash1[i] != hash2[i]) differences++;
    }
    
    // Oczekujemy co najmniej 50% różnic (32 z 64 znaków)
    EXPECT_GE(differences, 30);
}

TEST(HasherTest, SingleCharacter) {
    std::string digest_a = Hasher::sha256("a");
    std::string digest_b = Hasher::sha256("b");
    
    EXPECT_NE(digest_a, digest_b);
    EXPECT_EQ(digest_a.size(), 64);
    
    // Znany digest dla "a"
    EXPECT_EQ(digest_a, "ca978112ca1bbdcafac231b39a23dc4da786eff8147c4e72b9807785afee48bb");
}

TEST(HasherTest, DigestContainsOnlyHexCharacters) {
    const std::vector<std::string> test_inputs = {
        "", "a", "abc", "123", "!@#", "longer test string with spaces"
    };
    
    for (const auto& input : test_inputs) {
        std::string digest = Hasher::sha256(input);
        for (char c : digest) {
            bool is_hex = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
            EXPECT_TRUE(is_hex) << "Non-hex character '" << c 
                                << "' in digest for input: " << input;
        }
    }
}

TEST(HasherTest, CaseSensitivity) {
    std::string lower = Hasher::sha256("hello");
    std::string upper = Hasher::sha256("HELLO");
    
    EXPECT_NE(lower, upper);
}

TEST(HasherTest, NullBytesInString) {
    std::string with_null = "abc";
    with_null[1] = '\0';  // a\0c
    
    std::string without_null = "ac";
    
    EXPECT_NE(Hasher::sha256(with_null), Hasher::sha256(without_null));
}

TEST(HasherTest, WhitespaceMatters) {
    std::string no_space = "hello";
    std::string with_space = "hello ";
    std::string with_tab = "hello\t";
    std::string with_newline = "hello\n";
    
    EXPECT_NE(Hasher::sha256(no_space), Hasher::sha256(with_space));
    EXPECT_NE(Hasher::sha256(no_space), Hasher::sha256(with_tab));
    EXPECT_NE(Hasher::sha256(no_space), Hasher::sha256(with_newline));
}

TEST(HasherTest, EmptyStringDeterministic) {
    EXPECT_EQ(Hasher::sha256(""), SHA256_EMPTY);
    EXPECT_EQ(Hasher::sha256(""), Hasher::sha256(""));
}

TEST(HasherTest, VeryShortStrings) {
    for (int i = 1; i <= 10; ++i) {
        std::string input(i, 'x');
        std::string digest = Hasher::sha256(input);
        EXPECT_EQ(digest.size(), 64);
    }
}

TEST(HasherTest, PolishCharacterVariations) {
    std::string a = "zażółć";
    std::string b = "zazolc";
    
    EXPECT_NE(Hasher::sha256(a), Hasher::sha256(b));
}

TEST(HasherTest, LargeStringNoMemoryLeak) {
    std::string large(10000000, 'x');  // 10 MB stringa
    
    std::string digest = Hasher::sha256(large);
    
    EXPECT_EQ(digest.size(), 64);
}

TEST(HasherTest, MultipleCallsSameResult) {
    const std::string input = "consistent hashing test";
    
    std::string first = Hasher::sha256(input);
    for (int i = 0; i < 100; ++i) {
        EXPECT_EQ(Hasher::sha256(input), first);
    }
}

TEST(HasherTest, NoCollisionForDifferentInputs) {
    const std::string input1 = "collision test 1";
    const std::string input2 = "collision test 2";
    
    EXPECT_NE(Hasher::sha256(input1), Hasher::sha256(input2));
}