// Authors: Alesia Filinkova, Diana Pelin
// Description: Implementation of Hasher::sha256 using OpenSSL's
// one-shot SHA256() helper. Output format is the canonical lowercase
// hex digest used by RefreshEngine to compare file fingerprints.

#include "Hasher.h"

#include <openssl/sha.h>

#include <iomanip>
#include <sstream>
#include <string>

std::string Hasher::sha256(const std::string& data) {
    unsigned char hash[SHA256_DIGEST_LENGTH];

    SHA256(reinterpret_cast<const unsigned char*>(data.c_str()),
           data.size(),
           hash);

    std::stringstream stream;

    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        stream << std::hex << std::setw(2) << std::setfill('0')
               << static_cast<int>(hash[i]);
    }

    return stream.str();
}
