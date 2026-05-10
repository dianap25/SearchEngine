// Autorzy: Alesia Filinkova, Diana Pelin
// Opis: Punkt wejścia procesu. Tworzy obiekt Application i przekazuje
// mu argv. Cała właściwa logika znajduje się w Application::run.

#include "Application.h"

int main(int argc, char** argv) {
    Application application;
    return application.run(argc, argv);
}
