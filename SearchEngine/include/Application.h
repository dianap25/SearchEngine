//Alesia Filinkova
//Diana Pelin

#pragma once

class Application {
public:
    int run(int argc, char** argv);

private:
    int runIndexCommand(const char* directoryPath);
    void printUsage() const;
};