#include "Application.h"
#include "Database.h"
#include "Scanner.h"
#include "SearchEngine.h"

#include <iostream>

void Application::run() {
    Database database;
    database.open("index.db");

    Scanner scanner;
    scanner.scan();

    SearchEngine searchEngine;
    searchEngine.search();

    std::cout << "Application skeleton started successfully.\n";
}