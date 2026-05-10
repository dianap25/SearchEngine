// Authors: Alesia Filinkova, Diana Pelin

#include <gtest/gtest.h>

#include "Hasher.h"

namespace {

// Reference SHA-256 digest of the empty string, in lowercase hex.
constexpr const char* SHA256_EMPTY =
    "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";

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
