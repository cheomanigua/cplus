#pragma once

#include <SDL3/SDL.h>

struct Input;

struct Circle
{
    SDL_FPoint position{};
    float radius{};
};

struct Rectangle
{
    SDL_FPoint position{};
    SDL_FPoint size{};

    SDL_FRect GetBounds() const;
};

void Update(
    Circle& circle,
    Rectangle& rectangle,
    const Input& input,
    float deltaTime);
