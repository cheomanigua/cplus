#include <SDL3/SDL.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <vector>

constexpr int WINDOW_WIDTH { 800 };
constexpr int WINDOW_HEIGHT  { 600 };

constexpr const char* MAP_FILE { "map.txt" };

std::vector<SDL_FRect> obstacles;

// --------------------------------------------------
// Save map
// --------------------------------------------------

bool SaveMap(const char* filename)
{
    std::ofstream file(filename);

    if (!file.is_open())
        return false;

    for (const auto& obstacle : obstacles)
    {
        file << obstacle.x << ' '
             << obstacle.y << ' '
             << obstacle.w << ' '
             << obstacle.h << '\n';
    }

    return file.good();
}

// --------------------------------------------------
// Load map
// --------------------------------------------------

bool LoadMap(const char* filename)
{
    std::ifstream file(filename);

    if (!file.is_open())
        return false;

    std::vector<SDL_FRect> loadedObstacles;

    SDL_FRect obstacle{};

    while (file >> obstacle.x
                >> obstacle.y
                >> obstacle.w
                >> obstacle.h)
    {
        if (obstacle.w <= 0.0f || obstacle.h <= 0.0f ||
            obstacle.x < 0.0f || obstacle.y < 0.0f ||
            obstacle.x + obstacle.w > WINDOW_WIDTH ||
            obstacle.y + obstacle.h > WINDOW_HEIGHT)
        {
            return false;
        }

        loadedObstacles.push_back(obstacle);
    }

    if (!file.eof())
        return false;

    obstacles = std::move(loadedObstacles);

    return true;
}

// --------------------------------------------------
// Mouse helpers
// --------------------------------------------------

bool IsMouseOverObstacle(const SDL_FPoint& mouse, const SDL_FRect& obstacle)
{
    return SDL_PointInRectFloat(&mouse, &obstacle);
}

void DeleteObstacleAt(const SDL_FPoint& mouse)
{
    auto it = std::find_if(
        obstacles.begin(),
        obstacles.end(),
        [&](const SDL_FRect& obstacle)
        {
            return IsMouseOverObstacle(mouse, obstacle);
        }
    );

    if (it != obstacles.end())
        obstacles.erase(it);
}

// --------------------------------------------------
// Rectangle creation
// --------------------------------------------------

SDL_FRect MakeRectangle(const SDL_FPoint& start, const SDL_FPoint& end)
{
    const float left { std::min(start.x, end.x) };
    const float top { std::min(start.y, end.y) };

    const float right { std::max(start.x, end.x) };
    const float bottom { std::max(start.y, end.y) };

    return { left, top, right - left, bottom - top };
}

// --------------------------------------------------
// Main
// --------------------------------------------------

int main()
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("SDL3 Map Editor", WINDOW_WIDTH, WINDOW_HEIGHT, 0);

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

    bool running { true };
    bool drawing { false };

    SDL_FPoint startPoint{};
    SDL_FPoint mousePoint{};

    // --------------------------------------------------
    // Game loop
    // --------------------------------------------------

    while (running)
    {
        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_EVENT_QUIT:
                    running = false;
                    break;

                // --------------------------------------
                // Keyboard
                // --------------------------------------

                case SDL_EVENT_KEY_DOWN:
                {
                    if (event.key.repeat)
                        break;

                    switch (event.key.key)
                    {
                        case SDLK_ESCAPE:
                            running = false;
                            break;

                        case SDLK_S:
                            if (SaveMap(MAP_FILE))
                                SDL_Log("Map saved: %s", MAP_FILE);
                            else
                                SDL_Log("Failed to save map!");
                            break;

                        case SDLK_L:
                            if (LoadMap(MAP_FILE))
                                SDL_Log("Map loaded: %s", MAP_FILE);
                            else
                                SDL_Log("Failed to load map!");
                            break;

                        case SDLK_C:
                            obstacles.clear();
                            SDL_Log("Map cleared");
                            break;
                    }

                    break;
                }

                // --------------------------------------
                // Mouse button down
                // --------------------------------------

                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                {
                    mousePoint = {event.button.x, event.button.y};

                    if (event.button.button == SDL_BUTTON_LEFT)
                    {
                        startPoint = mousePoint;
                        drawing = true;
                    }
                    else if (event.button.button == SDL_BUTTON_RIGHT)
                    {
                        DeleteObstacleAt(mousePoint);
                    }

                    break;
                }

                // --------------------------------------
                // Mouse button up
                // --------------------------------------

                case SDL_EVENT_MOUSE_BUTTON_UP:
                {
                    if (event.button.button == SDL_BUTTON_LEFT && drawing)
                    {
                        SDL_FPoint endPoint{event.button.x, event.button.y};
                        SDL_FRect obstacle = MakeRectangle(startPoint, endPoint);

                        // Ignore very small rectangles
                        if (obstacle.w >= 2.0f && obstacle.h >= 2.0f)
                        {
                            obstacles.push_back(obstacle);
                        }

                        drawing = false;
                    }

                    break;
                }
            }
        }

        // ----------------------------------------------
        // Mouse position
        // ----------------------------------------------

        float mouseX{}, mouseY{};
        SDL_GetMouseState(&mouseX, &mouseY);

        mousePoint = { mouseX, mouseY };

        // ----------------------------------------------
        // Preview rectangle
        // ----------------------------------------------

        SDL_FRect preview{};

        if (drawing)
        {
            preview = MakeRectangle(startPoint, mousePoint);
        }

        // ----------------------------------------------
        // Rendering
        // ----------------------------------------------

        SDL_SetRenderDrawColor(renderer, 25, 25, 30, 255);
        SDL_RenderClear(renderer);

        // Draw grid

        SDL_SetRenderDrawColor(renderer, 40, 40, 48, 255);

        constexpr int GRID_SIZE { 25 };

        for (int x = 0; x <= WINDOW_WIDTH; x += GRID_SIZE)
        {
            SDL_RenderLine(
                renderer,
                static_cast<float>(x), 0.0f,
                static_cast<float>(x),
                static_cast<float>(WINDOW_HEIGHT)
            );
        }

        for (int y = 0; y <= WINDOW_HEIGHT; y += GRID_SIZE)
        {
            SDL_RenderLine(
                renderer,
                0.0f, static_cast<float>(y),
                static_cast<float>(WINDOW_WIDTH),
                static_cast<float>(y)
            );
        }

        // Draw obstacles

        for (const auto& obstacle : obstacles)
        {
            if (IsMouseOverObstacle(mousePoint, obstacle))
            {
                SDL_SetRenderDrawColor(renderer, 255, 190, 50, 255);
            }
            else
            {
                SDL_SetRenderDrawColor(renderer, 80, 100, 200, 255);
            }

            SDL_RenderFillRect(renderer, &obstacle);

            SDL_SetRenderDrawColor(renderer, 130, 150, 255, 255);
            SDL_RenderRect(renderer, &obstacle);
        }

        // Draw rectangle preview

        if (drawing)
        {
            SDL_SetRenderDrawColor(renderer, 80, 220, 120, 120);
            SDL_RenderFillRect(renderer, &preview);

            SDL_SetRenderDrawColor(renderer, 100, 255, 140, 255);
            SDL_RenderRect(renderer, &preview);
        }

        SDL_RenderPresent(renderer);
    }

    // --------------------------------------------------
    // Cleanup
    // --------------------------------------------------

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}

