// Authors: Alesia Filinkova, Diana Pelin
// Description: Result of a text extraction attempt (content and
// diagnostic error message).

#pragma once

#include <string>

/**
 * @brief Outcome of running a concrete Extractor implementation on a
 *        single file.
 */
struct ExtractResult {
    bool success = false;
    std::string content;
    std::string error_message;

    /**
     * @brief Builds a successful result.
     * @param text Extracted plain text.
     */
    static ExtractResult ok(std::string text) {
        return {true, std::move(text), ""};
    }

    /**
     * @brief Builds a failure result.
     * @param error Human-readable error description.
     */
    static ExtractResult fail(std::string error) {
        return {false, "", std::move(error)};
    }
};
