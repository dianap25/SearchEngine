// Authors: Alesia Filinkova, Diana Pelin
// Description: Top-level CLI dispatcher. Parses argv, maps commands
// (index/refresh/search-name/search-content) to the right service and
// prints a friendly usage screen when no command is provided.

#pragma once

#include <string>

/**
 * @brief Entry point of the SearchEngine CLI.
 */
class Application {
public:
    /**
     * @brief Run the CLI with command-line arguments.
     * @param argc argv count.
     * @param argv argv strings.
     * @return Process exit code.
     */
    int run(int argc, char** argv);

private:
    int runIndexCommand(const char* directory_path);
    int handleRefresh(const std::string& path);
    int handleSearchName(const std::string& phrase);
    int handleSearchContent(const std::string& word);
    std::string normalize(const std::string& input);
    void printUsage() const;
};
