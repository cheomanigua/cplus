#pragma once

#include <SDL3/SDL.h>

#include <memory>

#include "core/logging/logger.hpp"

class Simulation;

class Application
{
public:
    Application();
    ~Application();

	// Prevents copy construction:
	// Application app1;
	// Application app2 = app1; // not possible
    Application(const Application&) = delete;

	// Prevents copy assigment:
	// Application app1;
	// Application app2;
	// app2 = app1; // not possible
    Application& operator=(const Application&) = delete;

    bool initialize();
    void run();
    void shutdown();

private:
    Logger logger_;
    std::unique_ptr<Simulation> simulation_;

    SDL_Window* window_ = nullptr;

    bool sdlInitialized_ = false;
    bool initialized_ = false;
    bool running_ = false;
};

