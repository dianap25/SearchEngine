//Alesia Filinkova
//Diana Pelin

#pragma once

#include <string>

struct sqlite3;

class Database {
public:
    Database() = default;
    ~Database();

    bool open(const std::string& path);
    bool initializeSchema();

private:
    sqlite3* db_ = nullptr;
};