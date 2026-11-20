#include <SDL3/SDL.h>

constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 600;

int main()
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "SDL3 Rectangle Collision - C++20",
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        0
    );

    if (!window)
    {
        SDL_Log("Window creation failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);

    if (!renderer)
    {
        SDL_Log("Renderer creation failed: %s", SDL_GetError());

        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_SetRenderVSync(renderer, 1);

    // Player definition

    // Obstacles definition

    bool running = true;

    Uint64 lastTime = SDL_GetTicks();

    while (running)
    {
        // Delta time
        const Uint64 currentTime = SDL_GetTicks();
        const float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        // Events
        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
                running = false;
        }

        // Keyboard

		// Normalize diagonal movement if moving horizontally & vertically

		// Move player

        // Window boundaries

        // Mouse position

        // Rectangle collision detection

        // Point-in-rectangle detection

        // Rendering
        SDL_SetRenderDrawColor(renderer, 25, 25, 30, 255);
        SDL_RenderClear(renderer);

        // Draw obstacles
            // Mouse hovering over obstacle?

        // Draw player

        SDL_RenderPresent(renderer);
    }

    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}

