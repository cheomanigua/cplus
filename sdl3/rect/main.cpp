#include <SDL3/SDL.h>

#include <array>
#include <cstdint>

constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 600;

constexpr float PLAYER_SPEED = 300.0f;

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

    // Player
    SDL_FRect player{ 50.0f, 50.0f, 50.0f, 50.0f };

    // Obstacles
    constexpr std::array obstacles{
        SDL_FRect{ 200.0f, 100.0f, 150.0f, 100.0f },
        SDL_FRect{ 400.0f, 250.0f, 100.0f, 150.0f },
        SDL_FRect{ 600.0f, 350.0f, 150.0f, 100.0f },
        SDL_FRect{ 150.0f, 400.0f, 200.0f, 50.0f }
    };

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
        const bool* keys = SDL_GetKeyboardState(nullptr);
		SDL_FPoint direction{
			static_cast<float>(keys[SDL_SCANCODE_RIGHT] - keys[SDL_SCANCODE_LEFT]),
			static_cast<float>(keys[SDL_SCANCODE_DOWN] - keys[SDL_SCANCODE_UP])
		};

		// Normalize diagonal movement if moving horizontally & vertically
		if (direction.x != 0.0f && direction.y != 0.0f)
        {
            direction.x *= 0.70710678f;
            direction.y *= 0.70710678f;
        }

		// Move player
		player.x += direction.x * PLAYER_SPEED * deltaTime;
		player.y += direction.y * PLAYER_SPEED * deltaTime;


        // Window boundaries
        if (player.x < 0.0f)
            player.x = 0.0f;

        if (player.y < 0.0f)
            player.y = 0.0f;

        if (player.x + player.w > WINDOW_WIDTH)
            player.x = WINDOW_WIDTH - player.w;

        if (player.y + player.h > WINDOW_HEIGHT)
            player.y = WINDOW_HEIGHT - player.h;

        // Mouse position
        float mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);

        const SDL_FPoint mousePoint{
            mouseX, mouseY
        };

        // Rectangle collision detection
        bool collision = false;

        for (const auto& obstacle : obstacles)
        {
            if (SDL_HasRectIntersectionFloat(&player, &obstacle))
            {
                collision = true;
                break;
            }
        }

        // Point-in-rectangle detection
        bool mouseOverObstacle = false;

        for (const auto& obstacle : obstacles)
        {
            if (SDL_PointInRectFloat(&mousePoint, &obstacle))
            {
                mouseOverObstacle = true;
                break;
            }
        }

        // Rendering
        SDL_SetRenderDrawColor(renderer, 25, 25, 30, 255);
        SDL_RenderClear(renderer);

        // Draw obstacles
        for (const auto& obstacle : obstacles)
        {
            // Mouse hovering over obstacle?
            if (SDL_PointInRectFloat(&mousePoint, &obstacle))
            {
                SDL_SetRenderDrawColor(
                    renderer, 255, 200, 50, 255
                );
            }
            else
            {
                SDL_SetRenderDrawColor(
                    renderer, 80, 100, 200, 255
                );
            }

            SDL_RenderFillRect(renderer, &obstacle);
        }

        // Draw player
        if (collision)
            SDL_SetRenderDrawColor(renderer, 255, 60, 60, 255);
        else
            SDL_SetRenderDrawColor(renderer, 60, 220, 100, 255);

        SDL_RenderFillRect(renderer, &player);

        SDL_RenderPresent(renderer);
    }

    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}

