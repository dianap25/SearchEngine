// Autorzy: Alesia Filinkova, Diana Pelin
// Opis: Opakowanie RAII na połączenie z bazą SQLite.
// Posiada uchwyt sqlite3 trzymany w std::unique_ptr z własnym
// deleterem, udostępnia metodę connection() dla klasy Repository
// oraz odpowiada za inicjalizację schematu używanego przez polecenia index/refresh.

#pragma once

#include <memory>
#include <string>

struct sqlite3;

/**
 * @brief Posiada połączenie z SQLite oraz tworzy/migruje schemat.
 *
 * Połączenie jest trzymane w std::unique_ptr z własnym deleterem,
 * dzięki czemu zamknięcie następuje automatycznie, gdy obiekt
 * Database wychodzi z zakresu. Klasa jest jawnie niekopiowalna;
 * konstrukcja przenosząca i przypisanie przenoszące są domyślne,
 * więc Database może być zwracany z funkcji wytwórczych.
 */
class Database {
public:
    Database();

    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    Database(Database&&) = default;
    Database& operator=(Database&&) = default;

    /**
     * @brief Otwiera plik bazy SQLite (lub ":memory:" w testach).
     * @param path Ścieżka do pliku albo specjalny URI SQLite.
     * @return true jeśli połączenie zostało otwarte poprawnie.
     */
    bool open(const std::string& path);

    /**
     * @brief Tworzy schemat bazy i wykonuje oczekujące migracje.
     * @return true jeśli schemat jest gotowy do użycia.
     */
    bool initializeSchema();

    /**
     * @brief Surowy uchwyt połączenia używany przez Repository.
     * @return Wskaźnik własnościowy tego obiektu Database; nie należy
     *         go zwalniać.
     */
    sqlite3* connection();

private:
    using SqliteHandle = std::unique_ptr<sqlite3, void (*)(sqlite3*)>;

    bool executeSql(const std::string& sql);
    static SqliteHandle makeHandle(sqlite3* raw = nullptr);

    SqliteHandle db_;
};
