#pragma once

#include <string>

struct ExtractResult {
    bool success;
    std::string content;
    std::string errorMessage;

    static ExtractResult ok(std::string text) {
        return {true, std::move(text), ""};
    }

    static ExtractResult fail(std::string error) {
        return {false, "", std::move(error)};
    }
};