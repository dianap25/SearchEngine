// Authors: Alesia Filinkova, Diana Pelin
// Description: Legacy directory-walking search class. Currently only
// referenced by the build target; removed in a follow-up commit once
// the CLI no longer needs it.

#pragma once

#include <string>

/**
 * @brief Pre-index "live" directory search. Kept for compatibility.
 */
class SearchEngine {
public:
    void search(const std::string& root_path, const std::string& phrase);
};
