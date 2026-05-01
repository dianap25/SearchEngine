//Alesia Filinkova
//Diana Pelin

#pragma once

#include <string>

class Application {
public:
    int run(int argc, char** argv);

private:
    int runIndexCommand(const char* directoryPath);
    int handleRefresh(const std::string& path);
    int handleSearchName(const std::string& phrase);
    int handleSearchContent(const std::string& word);
    std::string normalize(const std::string& input);
    void printUsage() const;
};