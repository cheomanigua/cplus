#include "client/application.hpp"

#include <cstdlib>

int main()
{
    Application application;

    if (!application.initialize())
    {
        return EXIT_FAILURE;
    }

    application.run();
    application.shutdown();

    return EXIT_SUCCESS;
}

