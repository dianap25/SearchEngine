// Authors: Alesia Filinkova, Diana Pelin
// Description: Tiny SHA-256 helper around OpenSSL. Used by
// IndexService and RefreshEngine to compute a content fingerprint so
// refresh can skip unchanged files.

#pragma once

#include <string>

/**
 * @brief Stateless wrapper that returns the SHA-256 hex digest of a
 *        string buffer.
 */
class Hasher {
public:
    /**
     * @brief Compute the SHA-256 hex digest of @p data.
     * @param data Input buffer (treated as raw bytes).
     * @return Lowercase hex digest, 64 characters long.
     */
    static std::string sha256(const std::string& data);
};
