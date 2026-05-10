// Autorzy: Alesia Filinkova, Diana Pelin
// Opis: Implementacja Hasher::sha256 oparta na jednorazowej funkcji
// SHA256() z OpenSSL. Wynik to kanoniczny szesnastkowy skrót w
// małych literach, używany przez RefreshEngine do porównywania
// fingerprintów plików.

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
