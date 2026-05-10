// Authors: Alesia Filinkova, Diana Pelin
// Description: Parses argv, maps commands
// (index/refresh/search-name/search-content) to the relevant services
// and prints usage when no command is supplied.

#pragma once

#include <string>

/**
 * @brief CLI entry point for the SearchEngine application.
 */
class Application {
public:
    /**
     * @brief Runs the CLI with the given command-line arguments.
     * @param argc Number of arguments in argv.
     * @param argv Command-line arguments.
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
