//Alesia Filinkova
//Diana Pelin

#include "Application.h"
#include "Database.h"
#include "SearchEngine.h"

#include <iostream>

void Application::run() {
    Database database;

    if (!database.open("index.db")) {
        std::cerr << "Could not open database\n";
        return;
    }

    if (!database.initializeSchema()) {
        std::cerr << "Could not initialize database schema\n";
        return;
    }

    SearchEngine searchEngine;
    searchEngine.search("../sample_data", "example");

    std::cout << "Application skeleton started successfully.\n";
}