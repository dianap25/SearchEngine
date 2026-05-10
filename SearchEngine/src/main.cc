// Authors: Alesia Filinkova, Diana Pelin
// Description: Process entry point: builds an Application instance
// and forwards argc/argv to Application::run.


#include "Application.h"

int main(int argc, char** argv) {
    Application application;
    return application.run(argc, argv);
}
