// Authors: Alesia Filinkova, Diana Pelin
// Description: Process entry point. Constructs the Application object
// and forwards argv to it. All real logic lives in Application::run.

#include "Application.h"

int main(int argc, char** argv) {
    Application application;
    return application.run(argc, argv);
}
