// Authors: Alesia Filinkova, Diana Pelin
// Description: Computes SHA-256 on top of OpenSSL. Used by
// IndexService and RefreshEngine to derive a content fingerprint, so
// that refresh can skip unchanged files.

#pragma once

#include <string>

/**
 * @brief Stateless wrapper that returns a hexadecimal SHA-256 digest
 *        of a string buffer.
 */
class Hasher {
public:
    /**
     * @brief Computes the hexadecimal SHA-256 digest of @p data.
     * @param data Input buffer (treated as raw bytes).
     * @return Digest as a 64-character hex string (lowercase).
     */
    static std::string sha256(const std::string& data);
};
