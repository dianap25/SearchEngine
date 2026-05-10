// Authors: Alesia Filinkova, Diana Pelin
// Description: Result of a text-extraction attempt. Carries both the
// extracted content and a diagnostic message so callers can keep
// using return-code-style error handling instead of exceptions.

#pragma once

#include <string>

/**
 * @brief Outcome of running an Extractor against a file.
 */
struct ExtractResult {
    bool success = false;
    std::string content;
    std::string error_message;

    /**
     * @brief Build a successful result.
     * @param text Extracted text.
     */
    static ExtractResult ok(std::string text) {
        return {true, std::move(text), ""};
    }

    /**
     * @brief Build a failure result.
     * @param error Human-readable description of the failure.
     */
    static ExtractResult fail(std::string error) {
        return {false, "", std::move(error)};
    }
};
