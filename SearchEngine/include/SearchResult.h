// Authors: Alesia Filinkova, Diana Pelin
// Description: Plain data describing a single search hit returned by
// Repository::searchByName / searchByContent and consumed by the CLI
// formatter.

#pragma once

#include <string>

/**
 * @brief Single hit produced by name or content search.
 */
struct SearchResult {
    std::string path;
    std::string name;
    int occurrences = 0;
    std::string context;
};
