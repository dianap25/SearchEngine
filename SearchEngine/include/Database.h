#pragma once

#include <string>

class Database {
public:
    bool open(const std::string& path);
};