#include "draw.h"


void DrawCircleOutline(
    SDL_Renderer* renderer,
    SDL_FPoint center,
    float radius)
{
    const int cx = static_cast<int>(center.x);
    const int cy = static_cast<int>(center.y);

    int x = static_cast<int>(radius);
    int y = 0;

    int error = 1 - x;

    while (x >= y)
    {
        SDL_RenderPoint(renderer, cx + x, cy + y);
        SDL_RenderPoint(renderer, cx + y, cy + x);
        SDL_RenderPoint(renderer, cx - y, cy + x);
        SDL_RenderPoint(renderer, cx - x, cy + y);

        SDL_RenderPoint(renderer, cx - x, cy - y);
        SDL_RenderPoint(renderer, cx - y, cy - x);
        SDL_RenderPoint(renderer, cx + y, cy - x);
        SDL_RenderPoint(renderer, cx + x, cy - y);

        ++y;

        if (error <= 0)
        {
            error += 2 * y + 1;
        }
        else
        {
            --x;
            error += 2 * (y - x) + 1;
        }
    }
}


void DrawRectangleOutline(
    SDL_Renderer* renderer,
    const SDL_FRect& rect)
{
    SDL_RenderRect(renderer, &rect);
}
