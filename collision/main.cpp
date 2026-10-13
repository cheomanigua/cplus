#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "collision.h"
#include "draw.h"
#include "game.h"
#include "input.h"
#include "text.h"

namespace
{
    constexpr int SCREEN_WIDTH = 800;
    constexpr int SCREEN_HEIGHT = 450;

    constexpr float MAX_DELTA_TIME = 0.1f;
    constexpr double TARGET_FRAME_TIME = 1.0 / 60.0;

    constexpr float FONT_SIZE = 20.0f;
    constexpr const char* FONT_PATH = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";

    constexpr SDL_Color DARK_GRAY{80, 80, 80, 255};
    constexpr SDL_Color RED{230, 41, 55, 255};
    constexpr SDL_Color BLUE{0, 121, 241, 255};
}

int main()
{
    // Initialize SDL
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    // Initialize SDL_ttf
    if (!TTF_Init())
    {
        SDL_Log("TTF_Init failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // ------------------------------------------------------------
    // Create window and renderer
    // ------------------------------------------------------------

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    if (!SDL_CreateWindowAndRenderer("SDL3 Collision Example", SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer))
    {
        SDL_Log("SDL_CreateWindowAndRenderer failed: %s", SDL_GetError());

        TTF_Quit();
        SDL_Quit();

        return 1;
    }

    // ------------------------------------------------------------
    // Load font
    // ------------------------------------------------------------

    TTF_Font* font = TTF_OpenFont(FONT_PATH, FONT_SIZE);

    if (!font)
    {
        SDL_Log("TTF_OpenFont failed: %s", SDL_GetError());

        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);

        TTF_Quit();
        SDL_Quit();

        return 1;
    }

    // Text cache
    TextCache textCache(renderer, font);

    // Game objects
    Circle circle1{{400.0f, 215.0f}, 20.0f};
    Circle circle2{{500.0f, 215.0f}, 40.0f};

    Rectangle rectangle1{{250.0f, 200.0f}, {30.0f, 30.0f}};
    Rectangle rectangle2{{300.0f, 200.0f}, {30.0f, 30.0f}};

    // Timing
    const Uint64 performanceFrequency = SDL_GetPerformanceFrequency();
    Uint64 previousCounter = SDL_GetPerformanceCounter();

    // ------------------------------------------------------------
    // Main loop
    // ------------------------------------------------------------

    bool running = true;

    while (running)
    {
        // --------------------------------------------------------
        // Start frame
        // --------------------------------------------------------

        const Uint64 frameStart = SDL_GetPerformanceCounter();

        // --------------------------------------------------------
        // Calculate actual delta time
        // --------------------------------------------------------

        double deltaTime = static_cast<double>(frameStart - previousCounter) / static_cast<double>(performanceFrequency);
        previousCounter = frameStart;

        if (deltaTime > MAX_DELTA_TIME)
        {
            deltaTime = MAX_DELTA_TIME;
        }

        const float frameDeltaTime = static_cast<float>(deltaTime);

        // --------------------------------------------------------
        // Events
        // --------------------------------------------------------

        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_EVENT_QUIT:
                    running = false;
                    break;

                case SDL_EVENT_KEY_DOWN:
                    if (event.key.key == SDLK_ESCAPE)
                    {
                        running = false;
                    }
                    break;

                default:
                    break;
            }
        }

        if (!running)
        {
            break;
        }

        // Input
        const Input input = GetInput();

        // Update
        Update(circle1, rectangle1, input, frameDeltaTime);

        // Calculate bounds once
        const SDL_FRect rectangle1Bounds = rectangle1.GetBounds();
        const SDL_FRect rectangle2Bounds = rectangle2.GetBounds();

        // --------------------------------------------------------
        // Collision detection
        // --------------------------------------------------------

        const bool hasCollidedPC = CheckPointCircleCollision(
            circle1.position,
            circle2.position,
            circle1.radius
        );

        const bool hasCollidedPR = CheckPointRectangleCollision(
            circle2.position,
            rectangle1Bounds
        );

        const bool hasCollidedCC = CheckCircleCircleCollision(
            circle1.position,
            circle1.radius,
            circle2.position,
            circle2.radius
        );

        const bool hasCollidedRR = CheckRectangleRectangleCollision(
            rectangle1Bounds,
            rectangle2Bounds
        );

        const bool hasCollidedCR = CheckCircleRectangleCollision(
            circle1.position,
            circle1.radius,
            rectangle2Bounds
        );

        const bool hasCollidedRC = CheckCircleRectangleCollision(
            circle2.position,
            circle2.radius,
            rectangle1Bounds
        );

        // --------------------------------------------------------
        // Rendering
        // --------------------------------------------------------

        SDL_SetRenderDrawColor(renderer, 245, 245, 245, 255);
        SDL_RenderClear(renderer);

        // Circle 1 - blue
        SDL_SetRenderDrawColor(renderer, BLUE.r, BLUE.g, BLUE.b, BLUE.a);
        DrawCircleOutline(renderer, circle1.position, circle1.radius);

        // Small center marker for circle 2
        DrawCircleOutline(renderer, circle2.position, circle2.radius / 30.0f);

        // Circle 2 - red
        SDL_SetRenderDrawColor(renderer, RED.r, RED.g, RED.b, RED.a);
        DrawCircleOutline(renderer, circle2.position, circle2.radius);

        // Rectangle 1 - blue
        SDL_SetRenderDrawColor(renderer, BLUE.r, BLUE.g, BLUE.b, BLUE.a);
        DrawRectangleOutline(renderer, rectangle1Bounds);

        // Rectangle 2 - red
        SDL_SetRenderDrawColor(renderer, RED.r, RED.g, RED.b, RED.a);
        DrawRectangleOutline(renderer, rectangle2Bounds);

        // Text
        textCache.Draw("Move the blue circle and blue rectangle with arrow keys", 10.0f, 10.0f, DARK_GRAY);

        const char* pointCircleText = hasCollidedPC ? "PointInCircle: YES" : "PointInCircle: NO";
        textCache.Draw(pointCircleText, 10.0f, 35.0f, hasCollidedPC ? RED : DARK_GRAY);

        const char* pointRectText = hasCollidedPR ? "PointInRect: YES" : "PointInRect: NO";
        textCache.Draw(pointRectText, 10.0f, 60.0f, hasCollidedPR ? RED : DARK_GRAY);

        const char* circleCircleText = hasCollidedCC ? "CirclevsCircle: YES" : "CirclevsCircle: NO";
        textCache.Draw(circleCircleText, 10.0f, 85.0f, hasCollidedCC ? RED : DARK_GRAY);

        const char* rectRectText = hasCollidedRR ? "RectvsRect: YES" : "RectvsRect: NO";
        textCache.Draw(rectRectText, 10.0f, 110.0f, hasCollidedRR ? RED : DARK_GRAY);

        const char* circleRectText = hasCollidedCR ? "CirclevsRect: YES" : "CirclevsRect: NO";
        textCache.Draw(circleRectText, 10.0f, 135.0f, hasCollidedCR ? RED : DARK_GRAY);

        const char* rectCircleText = hasCollidedRC ? "RectvsCircle: YES" : "RectvsCircle: NO";
        textCache.Draw(rectCircleText, 10.0f, 160.0f, hasCollidedRC ? RED : DARK_GRAY);

        // Present
        SDL_RenderPresent(renderer);

        // Frame-rate limiter
        const Uint64 frameEnd = SDL_GetPerformanceCounter();
        const double frameTime = static_cast<double>(frameEnd - frameStart) / static_cast<double>(performanceFrequency);
        const double remaining = TARGET_FRAME_TIME - frameTime;

        if (remaining > 0.0)
        {
            const Uint32 delayMilliseconds = static_cast<Uint32>(remaining * 1000.0);

            if (delayMilliseconds > 0)
            {
                SDL_Delay(delayMilliseconds);
            }
        }
    }

    // ------------------------------------------------------------
    // Cleanup
    // ------------------------------------------------------------

    TTF_CloseFont(font);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    TTF_Quit();
    SDL_Quit();

    return 0;
}
