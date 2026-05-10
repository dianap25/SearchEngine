// Autorzy: Alesia Filinkova, Diana Pelin
// Opis: Warstwa dostępu do bazy. Wszystkie odczyty i zapisy do tabel
// files, file_texts, terms i postings są wykonywane tutaj, aby
// pozostała część silnika nie musiała znać surowego SQL.

#pragma once

#include "FileMetadata.h"
#include "SearchResult.h"

#include <optional>
#include <string>
#include <utility>
#include <vector>

struct sqlite3;

/**
 * @brief Warstwa trwałości dla tabel files, file_texts, terms oraz
 *        postings.
 *
 * Repository nie posiada na własność połączenia; to wywołujący
 * (zwykle właściciel klasy Database) gwarantuje, że uchwyt sqlite3*
 * przeżyje obiekt Repository.
 */
class Repository {
public:
    explicit Repository(sqlite3* db);

    Repository(const Repository&) = delete;
    Repository& operator=(const Repository&) = delete;
    Repository(Repository&&) = default;
    Repository& operator=(Repository&&) = default;

    /** @brief Rozpoczyna transakcję SQL. @return true jeśli się powiedzie. */
    bool beginTransaction();
    /** @brief Zatwierdza otwartą transakcję SQL. */
    bool commitTransaction();
    /** @brief Wycofuje otwartą transakcję SQL. */
    bool rollbackTransaction();

    /**
     * @brief Wstawia lub aktualizuje wiersz w tabeli files.
     * @param metadata Metadane pliku (id może być zerowe przy
     *                 wstawianiu nowego rekordu).
     * @return Liczbowe id wiersza albo -1 w razie niepowodzenia.
     */
    int saveFileMetadata(const FileMetadata& metadata);

    /**
     * @brief Zastępuje zapamiętany tekst pliku w cache.
     * @param file_id Id pliku w bazie.
     * @param content Wydobyty tekst do zapisania.
     */
    bool saveFileText(int file_id, const std::string& content);

    /**
     * @brief Dopisuje pojedynczą parę (term, position) do postings.
     *
     * W przypadku zapisu większej liczby postingów warto użyć
     * saveTermPositionsBatch — ta metoda przygotowuje dwa
     * zapytania na każde wywołanie.
     *
     * @param file_id Id pliku w bazie.
     * @param term Znormalizowany term.
     * @param position Pozycja tokenu w pliku.
     */
    [[deprecated("Dla plików z większą liczbą tokenów użyj saveTermPositionsBatch.")]]
    bool saveTermPosition(int file_id, const std::string& term, int position);

    /**
     * @brief Wstawia wszystkie postingi dla jednego pliku, używając
     *        prepared statementów oraz cache'a termów dla danego
     *        pliku.
     *
     * Przygotowuje statementy term-insert, term-select oraz
     * posting-insert dokładnie raz, a następnie iteruje po wektorze
     * @p tokens, bindując, wykonując i resetując każdy z nich.
     * Powtórzone termy w tym samym pliku są rozwiązywane przez
     * cache w pamięci, co eliminuje dodatkowe odpytywanie tabeli
     * terms.
     *
     * @param file_id Id pliku w bazie.
     * @param tokens Pary (znormalizowany term, pozycja) w kolejności
     *               występowania w dokumencie.
     * @return true jeśli każdy posting został zapisany poprawnie.
     */
    bool saveTermPositionsBatch(
        int file_id,
        const std::vector<std::pair<std::string, int>>& tokens
    );

    /** @brief Wyszukuje metadane pliku po dokładnej ścieżce. */
    std::optional<FileMetadata> findByPath(const std::string& path);
    /** @brief Wyszukuje wydobyty tekst po dokładnej ścieżce. */
    std::optional<std::string> findTextByPath(const std::string& path);
    /** @brief Zwraca metadane wszystkich aktualnie zaindeksowanych plików. */
    std::vector<FileMetadata> findAllFiles();

    /** @brief Usuwa wiersz z tabeli files (kaskaduje na postings/file_texts). */
    bool deleteFileByPath(const std::string& path);
    /** @brief Usuwa wszystkie postingi należące do jednego pliku. */
    bool deleteIndexForFile(int file_id);

    /** @brief Wyszukuje pliki po fragmencie nazwy (case-insensitive). */
    std::vector<SearchResult> searchByName(const std::string& phrase);
    /** @brief Wyszukuje pliki po dokładnym, znormalizowanym termie w treści. */
    std::vector<SearchResult> searchByContent(const std::string& term);

private:
    bool executeSql(const std::string& sql);
    int findOrCreateTerm(const std::string& term);

    sqlite3* db_;
};
