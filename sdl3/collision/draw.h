#pragma once

#include <SDL3/SDL.h>

void DrawCircleOutline(
    SDL_Renderer* renderer,
    SDL_FPoint center,
    float radius);

void DrawRectangleOutline(
    SDL_Renderer* renderer,
    const SDL_FRect& rect);
