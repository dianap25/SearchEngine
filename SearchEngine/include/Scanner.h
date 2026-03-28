#pragma once
#include <vector>
#include <string>

class Scanner {
public:
    std::vector<std::string> scan(const std::string& path);
private:
    bool isSupported(const std::string& path);
};