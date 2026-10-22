#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <random>

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include "flecs.h"

constexpr int SCREEN_WIDTH = 1280;
constexpr int SCREEN_HEIGHT = 720;
constexpr std::size_t ENTITY_COUNT = 200;
constexpr int SPRITE_SIZE = 32;

constexpr double TARGET_FPS = 60.0;
constexpr double TARGET_FRAME_TIME = 1.0 / TARGET_FPS;

constexpr const char* WINDOW_TITLE = "Flecs + SDL3";
constexpr const char* TEXTURE_PATH = "raybunny.png";

struct Position {
    float x{}, y{};
};

struct Velocity {
    float x{}, y{};
};

static void updateEntities(
    flecs::query<Position, Velocity>& query,
    float deltaTime,
    float maxX,
    float maxY)
{
    query.each([=](Position& pos, Velocity& vel) {
        pos.x += vel.x * deltaTime;
        pos.y += vel.y * deltaTime;

        if (pos.x < 0.0f) {
            pos.x = 0.0f;
            vel.x = std::abs(vel.x);
        } else if (pos.x > maxX) {
            pos.x = maxX;
            vel.x = -std::abs(vel.x);
        }

        if (pos.y < 0.0f) {
            pos.y = 0.0f;
            vel.y = std::abs(vel.y);
        } else if (pos.y > maxY) {
            pos.y = maxY;
            vel.y = -std::abs(vel.y);
        }
    });
}

static void drawEntities(
    SDL_Renderer* renderer,
    flecs::query<Position>& query,
    SDL_Texture* texture)
{
    query.each([&](const Position& pos) {
        const SDL_FRect destination{
            pos.x,
            pos.y,
            static_cast<float>(SPRITE_SIZE),
            static_cast<float>(SPRITE_SIZE)
        };

        SDL_RenderTexture(
            renderer,
            texture,
            nullptr,
            &destination
        );
    });
}

int main()
{
    // Initialize SDL3.
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        WINDOW_TITLE,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        0
    );

    if (!window) {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(
        window,
        nullptr
    );

    if (!renderer) {
        SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Disable VSync because we apply our own 60 FPS limiter.
    SDL_SetRenderVSync(renderer, 0);

    // Load raybunny.png from the working directory.
    SDL_Texture* texture = IMG_LoadTexture(
        renderer,
        TEXTURE_PATH
    );

    if (!texture) {
        SDL_Log(
            "Failed to load %s: %s",
            TEXTURE_PATH,
            SDL_GetError()
        );

        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);

    // Initialize Flecs.
    flecs::world world;

    std::mt19937 rng(std::random_device{}());

    std::uniform_real_distribution<float> xDist(
        0.0f,
        static_cast<float>(SCREEN_WIDTH - SPRITE_SIZE)
    );

    std::uniform_real_distribution<float> yDist(
        0.0f,
        static_cast<float>(SCREEN_HEIGHT - SPRITE_SIZE)
    );

    std::uniform_real_distribution<float> velocityDist(
        -200.0f,
        200.0f
    );

    // Create entities.
    for (std::size_t i = 0; i < ENTITY_COUNT; ++i) {
        world.entity()
            .set<Position>({xDist(rng), yDist(rng)})
            .set<Velocity>({
                velocityDist(rng),
                velocityDist(rng)
            });
    }

    // Cache queries once.
    auto movementQuery = world
        .query_builder<Position, Velocity>()
        .cached()
        .build();

    auto renderQuery = world
        .query_builder<Position>()
        .cached()
        .build();

    bool running = true;

    const Uint64 performanceFrequency =
        SDL_GetPerformanceFrequency();

    Uint64 previousCounter = SDL_GetPerformanceCounter();

    // FPS reporting.
    float fpsElapsed = 0.0f;
    std::uint32_t frames = 0;

    while (running) {
        // Start frame timer.
        const Uint64 frameStart = SDL_GetPerformanceCounter();

        // Process events.
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT ||
					event.type == SDL_EVENT_KEY_DOWN &&
					event.key.key == SDLK_ESCAPE) {
                running = false;
            }
        }

        if (!running) {
            break;
        }

        // Calculate delta time.
        const Uint64 currentCounter =
            SDL_GetPerformanceCounter();

        float deltaTime = static_cast<float>(
            static_cast<double>(
                currentCounter - previousCounter
            ) /
            static_cast<double>(performanceFrequency)
        );

        previousCounter = currentCounter;

        // Prevent large simulation jumps after a pause.
        deltaTime = std::min(deltaTime, 0.05f);

        // Update ECS.
        updateEntities(
            movementQuery,
            deltaTime,
            static_cast<float>(SCREEN_WIDTH - SPRITE_SIZE),
            static_cast<float>(SCREEN_HEIGHT - SPRITE_SIZE)
        );

        // Render.
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        drawEntities(renderer, renderQuery, texture);

        SDL_RenderPresent(renderer);

        // Update FPS in the window title once per second.
        ++frames;
        fpsElapsed += deltaTime;

        if (fpsElapsed >= 1.0f) {
            char title[128];

            std::snprintf(
                title,
                sizeof(title),
                "%s | %zu entities | %.1f FPS",
                WINDOW_TITLE,
                ENTITY_COUNT,
                static_cast<double>(frames) / fpsElapsed
            );

            SDL_SetWindowTitle(window, title);

            frames = 0;
            fpsElapsed = 0.0f;
        }

        // Limit frame rate to 60 FPS.
        const Uint64 frameEnd = SDL_GetPerformanceCounter();

        const double frameTime =
            static_cast<double>(frameEnd - frameStart) /
            static_cast<double>(performanceFrequency);

        if (frameTime < TARGET_FRAME_TIME) {
            const double remainingTime =
                TARGET_FRAME_TIME - frameTime;

            // Sleep for most of the remaining time.
            // SDL_Delay uses millisecond granularity.
            if (remainingTime > 0.002) {
                SDL_Delay(static_cast<Uint32>(
                    (remainingTime - 0.001) * 1000.0
                ));
            }
        }
    }

    // Cleanup.
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
