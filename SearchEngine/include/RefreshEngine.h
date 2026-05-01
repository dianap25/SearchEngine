//Alesia Filinkova
//Diana Pelin

#pragma once

#include <string>
#include "Database.h"

class RefreshEngine {
public:
    void rebuild(const std::string& rootPath, Database& db);
};