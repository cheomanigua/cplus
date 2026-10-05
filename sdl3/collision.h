#pragma once

#include <SDL3/SDL.h>

// Point vs circle
bool CheckPointCircleCollision(
    SDL_FPoint point,
    SDL_FPoint circleCenter,
    float circleRadius);

// Point vs rectangle
bool CheckPointRectangleCollision(
    SDL_FPoint point,
    const SDL_FRect& rectangle);

// Circle vs circle
bool CheckCircleCircleCollision(
    SDL_FPoint centerA,
    float radiusA,
    SDL_FPoint centerB,
    float radiusB);

// Rectangle vs rectangle
bool CheckRectangleRectangleCollision(
    const SDL_FRect& rectangleA,
    const SDL_FRect& rectangleB);

// Circle vs rectangle
bool CheckCircleRectangleCollision(
    SDL_FPoint circleCenter,
    float circleRadius,
    const SDL_FRect& rectangle);
