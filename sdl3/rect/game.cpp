
#include <SDL3/SDL.h>

#include <fstream>
#include <iostream>
#include <vector>

// --------------------------------------------------
// Configuration
// --------------------------------------------------

constexpr int WINDOW_WIDTH { 800 };
constexpr int WINDOW_HEIGHT { 600 };

constexpr float PLAYER_SPEED { 100.0f };

constexpr const char* MAP_FILE { "map.txt" };

// --------------------------------------------------
// Map
// --------------------------------------------------

bool LoadMap(const char* filename, std::vector<SDL_FRect>& obstacles)
{
    std::ifstream file(filename);

    if (!file.is_open())
    {
        std::cerr << "Failed to open map file: "
                  << filename << '\n';

        return false;
    }

    std::vector<SDL_FRect> loadedObstacles{};

    SDL_FRect obstacle{};

    while (file >> obstacle.x
                >> obstacle.y
                >> obstacle.w
                >> obstacle.h)
    {
        // Validate obstacle dimensions
        if (obstacle.w <= 0.0f ||
            obstacle.h <= 0.0f)
        {
            std::cerr << "Invalid obstacle dimensions!\n";
            return false;
        }

        // Validate obstacle position
        if (obstacle.x < 0.0f ||
            obstacle.y < 0.0f ||
            obstacle.x + obstacle.w > WINDOW_WIDTH ||
            obstacle.y + obstacle.h > WINDOW_HEIGHT)
        {
            std::cerr << "Obstacle outside map boundaries!\n";
            return false;
        }

        loadedObstacles.push_back(obstacle);
    }

    // Detect malformed or incomplete input
    if (!file.eof())
    {
        std::cerr << "Invalid map file format!\n";
        return false;
    }

    obstacles = std::move(loadedObstacles);

    std::cout << "Loaded "
              << obstacles.size()
              << " obstacles\n";

    return true;
}

// --------------------------------------------------
// Collision detection
// --------------------------------------------------

bool CheckCollision(const SDL_FRect& a, const SDL_FRect& b)
{
    return SDL_HasRectIntersectionFloat(&a, &b);
}

// --------------------------------------------------
// Player collision resolution
// --------------------------------------------------

void ResolveCollisions(
    SDL_FRect& player,
    const std::vector<SDL_FRect>& obstacles,
    float dx,
    float dy)
{
    // ----------------------------------------------
    // Horizontal movement
    // ----------------------------------------------

    player.x += dx;

    for (const auto& obstacle : obstacles)
    {
        const bool verticalOverlap =
            player.y < obstacle.y + obstacle.h &&
            player.y + player.h > obstacle.y;

        if (!verticalOverlap)
            continue;

        // Moving right
        if (dx > 0.0f &&
            player.x + player.w > obstacle.x &&
            player.x < obstacle.x)
        {
            player.x = obstacle.x - player.w;
        }
        // Moving left
        else if (dx < 0.0f &&
                 player.x < obstacle.x + obstacle.w &&
                 player.x + player.w > obstacle.x + obstacle.w)
        {
            player.x = obstacle.x + obstacle.w;
        }
    }

    // ----------------------------------------------
    // Vertical movement
    // ----------------------------------------------

    player.y += dy;

    for (const auto& obstacle : obstacles)
    {
        const bool horizontalOverlap =
            player.x < obstacle.x + obstacle.w &&
            player.x + player.w > obstacle.x;

        if (!horizontalOverlap)
            continue;

        // Moving down
        if (dy > 0.0f &&
            player.y + player.h > obstacle.y &&
            player.y < obstacle.y)
        {
            player.y = obstacle.y - player.h;
        }
        // Moving up
        else if (dy < 0.0f &&
                 player.y < obstacle.y + obstacle.h &&
                 player.y + player.h > obstacle.y + obstacle.h)
        {
            player.y = obstacle.y + obstacle.h;
        }
    }
}

// --------------------------------------------------
// Window boundaries
// --------------------------------------------------

void ResolveWindowBounds(SDL_FRect& player)
{
    if (player.x < 0.0f)
        player.x = 0.0f;

    if (player.y < 0.0f)
        player.y = 0.0f;

    if (player.x + player.w > WINDOW_WIDTH)
        player.x = WINDOW_WIDTH - player.w;

    if (player.y + player.h > WINDOW_HEIGHT)
        player.y = WINDOW_HEIGHT - player.h;
}

// --------------------------------------------------
// Collision state
// --------------------------------------------------

bool IsColliding(
    const SDL_FRect& player,
    const std::vector<SDL_FRect>& obstacles)
{
    for (const auto& obstacle : obstacles)
    {
        if (CheckCollision(player, obstacle))
            return true;
    }

    return false;
}

// --------------------------------------------------
// Mouse hover detection
// --------------------------------------------------

//bool IsMouseOverObstacle(
//    const SDL_FPoint& mouse,
//    const std::vector<SDL_FRect>& obstacles)
//{
//    for (const auto& obstacle : obstacles)
//    {
//        if (SDL_PointInRectFloat(&mouse, &obstacle))
//            return true;
//    }
//
//    return false;
//}

// --------------------------------------------------
// Rendering
// --------------------------------------------------

void Render(
    SDL_Renderer* renderer,
    const SDL_FRect& player,
    const std::vector<SDL_FRect>& obstacles,
    const SDL_FPoint& mouse,
    bool collision)
{
    // Background
    SDL_SetRenderDrawColor(
        renderer, 25, 25, 30, 255
    );

    SDL_RenderClear(renderer);

    // ----------------------------------------------
    // Draw obstacles
    // ----------------------------------------------

    for (const auto& obstacle : obstacles)
    {
        if (SDL_PointInRectFloat(&mouse, &obstacle))
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

    // ----------------------------------------------
    // Draw player
    // ----------------------------------------------

    if (collision)
    {
        SDL_SetRenderDrawColor(
            renderer, 255, 60, 60, 255
        );
    }
    else
    {
        SDL_SetRenderDrawColor(
            renderer, 60, 220, 100, 255
        );
    }

    SDL_RenderFillRect(renderer, &player);

    SDL_RenderPresent(renderer);
}

// --------------------------------------------------
// Main
// --------------------------------------------------

int main()
{
    // ----------------------------------------------
    // Initialize SDL
    // ----------------------------------------------

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    // ----------------------------------------------
    // Load map
    // ----------------------------------------------

    std::vector<SDL_FRect> obstacles;

    if (!LoadMap(MAP_FILE, obstacles))
    {
        SDL_Log("Could not load map. Starting with empty map.");
    }

    // ----------------------------------------------
    // Create window
    // ----------------------------------------------

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

    // ----------------------------------------------
    // Create renderer
    // ----------------------------------------------

    SDL_Renderer* renderer = SDL_CreateRenderer(
        window,
        nullptr
    );

    if (!renderer)
    {
        SDL_Log("Renderer creation failed: %s", SDL_GetError());

        SDL_DestroyWindow(window);
        SDL_Quit();

        return 1;
    }

	// Set FPS to monitor refresh rate
    SDL_SetRenderVSync(renderer, 1);

    // ----------------------------------------------
    // Player
    // ----------------------------------------------

    SDL_FRect player{50.0f, 50.0f, 20.0f, 20.0f};

    bool running = true;

    Uint64 lastTime = SDL_GetTicks();

    // ----------------------------------------------
    // Game loop
    // ----------------------------------------------

    while (running)
    {
        // ------------------------------------------
        // Delta time
        // ------------------------------------------

        const Uint64 currentTime = SDL_GetTicks();
        const float deltaTime = static_cast<float>(currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        // ------------------------------------------
        // Events
        // ------------------------------------------

        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
                running = false;
        }

        // ------------------------------------------
        // Keyboard input
        // ------------------------------------------

        const bool* keys = SDL_GetKeyboardState(nullptr);

        SDL_FPoint direction{
            static_cast<float>(
                keys[SDL_SCANCODE_RIGHT] - keys[SDL_SCANCODE_LEFT]),
            static_cast<float>(
                keys[SDL_SCANCODE_DOWN] - keys[SDL_SCANCODE_UP])
        };

        // Normalize diagonal movement
        if (direction.x != 0.0f && direction.y != 0.0f)
        {
            constexpr float NORMALIZATION = 0.70710678f;

            direction.x *= NORMALIZATION;
            direction.y *= NORMALIZATION;
        }

        // ------------------------------------------
        // Movement
        // ------------------------------------------

        const float dx = direction.x * PLAYER_SPEED * deltaTime;
        const float dy = direction.y * PLAYER_SPEED * deltaTime;

        ResolveCollisions(player, obstacles, dx, dy);
        ResolveWindowBounds(player);

        // ------------------------------------------
        // Collision detection
        // ------------------------------------------

        const bool collision = IsColliding(player, obstacles);

        // ------------------------------------------
        // Mouse
        // ------------------------------------------

        float mouseX{}, mouseY{};

        SDL_GetMouseState(&mouseX, &mouseY);

        const SDL_FPoint mousePoint{mouseX, mouseY};

        // ------------------------------------------
        // Rendering
        // ------------------------------------------

        Render(renderer, player, obstacles, mousePoint, collision);
    }

    // ----------------------------------------------
    // Cleanup
    // ----------------------------------------------

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}

