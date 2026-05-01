// Alesia Filinkova
// Diana Pelin

# pragma once

#include <openssl/sha.h>
#include <sstream>
#include <iomanip>
#include <string>

class Hasher {
public:
    static std::string sha256(const std::string& data);
};