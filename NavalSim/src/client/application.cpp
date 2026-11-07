
#include "client/application.hpp"
#include "simulation/simulation.hpp"

#include <exception>
#include <memory>

Application::Application() = default;

Application::~Application()
{
    shutdown();
}

bool Application::initialize()
{
    if (initialized_)
    {
        return true;
    }

    logger_.info("Initializing NavalSim...");

    // Initialize SDL3.
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        logger_.error(SDL_GetError());
        return false;
    }

    sdlInitialized_ = true;
    logger_.info("SDL3 initialized.");

    // Create the application window.
    window_ = SDL_CreateWindow(
        "NavalSim v0.1.0",
        1280,
        720,
        SDL_WINDOW_RESIZABLE
    );

    if (window_ == nullptr)
    {
        logger_.error(SDL_GetError());
        shutdown();
        return false;
    }

    logger_.info("Window created.");

    // Initialize the simulation.
    try
    {
        simulation_ = std::make_unique<Simulation>();

        if (!simulation_->initialize())
        {
            logger_.error("Failed to initialize simulation.");
            shutdown();
            return false;
        }
    }
    catch (const std::exception& exception)
    {
        logger_.error(exception.what());
        shutdown();
        return false;
    }

    logger_.info("Simulation initialized.");

    initialized_ = true;
    logger_.info("NavalSim initialized successfully.");

    return true;
}

void Application::run()
{
    if (!initialized_)
    {
        logger_.error("Cannot run: application is not initialized.");
        return;
    }

    logger_.info("Entering application loop.");

    running_ = true;

    while (running_)
    {
        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                running_ = false;
            }
        }

        // Temporary idle loop; simulation updates and rendering will be added later.
        SDL_Delay(16);
    }

    logger_.info("Application loop finished.");
}

void Application::shutdown()
{
    const bool hadResources =
        initialized_ ||
        sdlInitialized_ ||
        window_ != nullptr ||
        simulation_ != nullptr;

    running_ = false;

    // Shut down the simulation before releasing SDL resources.
    if (simulation_ != nullptr)
    {
        simulation_->shutdown();
        simulation_.reset();
    }

    // Destroy the application window.
    if (window_ != nullptr)
    {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }

    // Shut down SDL3.
    if (sdlInitialized_)
    {
        SDL_Quit();
        sdlInitialized_ = false;
    }

    initialized_ = false;

    if (hadResources)
    {
        logger_.info("NavalSim shutdown complete.");
    }
}

