//Alesia Filinkova
//Diana Pelin

#pragma once

#include "Database.h"
#include "IndexSummary.h"

#include <string>

class IndexService {
public:
    explicit IndexService(Database& database);

    IndexSummary indexDirectory(const std::string& rootPath);

private:
    Database& database_;
};