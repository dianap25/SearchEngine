// Authors: Alesia Filinkova, Diana Pelin
// Description: Data structure describing a single hit returned by
// Repository::searchByName / searchByContent and consumed by the CLI
// formatting layer.

#pragma once

#include <string>

/**
 * @brief A single hit returned by either name-based or
 *        content-based search.
 */
struct SearchResult {
    std::string path;
    std::string name;
    int occurrences = 0;
    std::string context;
};
