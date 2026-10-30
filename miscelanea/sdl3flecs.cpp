#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <random>

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include "flecs.h"

constexpr int SCREEN_WIDTH = 1280;
constexpr int SCREEN_HEIGHT = 720;
constexpr std::size_t ENTITY_COUNT = 200;
constexpr int SPRITE_SIZE = 32;

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

    // Enable VSync.
    if (!SDL_SetRenderVSync(renderer, 1)) {
        SDL_Log("Could not enable VSync: %s", SDL_GetError());
    }

    // Load texture.
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

    const Uint64 performanceFrequency = SDL_GetPerformanceFrequency();

    Uint64 previousCounter = SDL_GetPerformanceCounter();

    // FPS reporting.
    float fpsElapsed = 0.0f;
    std::uint32_t frames = 0;

    while (running) {

        // Process events.
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT ||
                (event.type == SDL_EVENT_KEY_DOWN &&
                 event.key.key == SDLK_ESCAPE)) {
                running = false;
            }
        }

        if (!running) {
            break;
        }

        // Calculate actual delta time.
        const Uint64 currentCounter = SDL_GetPerformanceCounter();

        const float rawDeltaTime = static_cast<float>(
            static_cast<double>(currentCounter - previousCounter) /
            static_cast<double>(performanceFrequency)
        );

        previousCounter = currentCounter;

        // Prevent large simulation jumps after a pause.
        const float deltaTime = std::min(rawDeltaTime, 0.05f);

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
        fpsElapsed += rawDeltaTime;

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
    }

    // Cleanup.
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

	return 0;
}
