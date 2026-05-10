// Autorzy: Alesia Filinkova, Diana Pelin
// Opis: Główny dyspozytor CLI. Parsuje argv, mapuje polecenia
// (index/refresh/search-name/search-content) na odpowiednie usługi i
// wyświetla przyjazny ekran pomocy, gdy nie podano żadnego
// polecenia.

#pragma once

#include <string>

/**
 * @brief Punkt wejścia CLI aplikacji SearchEngine.
 */
class Application {
public:
    /**
     * @brief Uruchamia CLI z argumentami wiersza poleceń.
     * @param argc Liczba argumentów argv.
     * @param argv Argumenty wiersza poleceń.
     * @return Kod wyjścia procesu.
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
