//Alesia Filinkova
//Diana Pelin

#pragma once

#include <string>
#include "Database.h"

class RefreshEngine {
public:
    void refresh(const std::string& rootPath, Database& db);
};