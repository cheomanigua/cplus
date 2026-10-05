#include "collision.h"

#include <algorithm>


bool CheckPointCircleCollision(
    SDL_FPoint point,
    SDL_FPoint circleCenter,
    float circleRadius)
{
    const float dx = point.x - circleCenter.x;
    const float dy = point.y - circleCenter.y;

    const float distanceSquared = dx * dx + dy * dy;
    const float radiusSquared = circleRadius * circleRadius;

    return distanceSquared <= radiusSquared;
}


bool CheckPointRectangleCollision(
    SDL_FPoint point,
    const SDL_FRect& rectangle)
{
    return point.x >= rectangle.x &&
           point.x < rectangle.x + rectangle.w &&
           point.y >= rectangle.y &&
           point.y < rectangle.y + rectangle.h;
}


bool CheckCircleCircleCollision(
    SDL_FPoint centerA,
    float radiusA,
    SDL_FPoint centerB,
    float radiusB)
{
    const float dx = centerA.x - centerB.x;
    const float dy = centerA.y - centerB.y;

    const float distanceSquared = dx * dx + dy * dy;
    const float radiusSum = radiusA + radiusB;

    return distanceSquared <= radiusSum * radiusSum;
}


bool CheckRectangleRectangleCollision(
    const SDL_FRect& rectangleA,
    const SDL_FRect& rectangleB)
{
    return rectangleA.x < rectangleB.x + rectangleB.w &&
           rectangleA.x + rectangleA.w > rectangleB.x &&
           rectangleA.y < rectangleB.y + rectangleB.h &&
           rectangleA.y + rectangleA.h > rectangleB.y;
}


bool CheckCircleRectangleCollision(
    SDL_FPoint circleCenter,
    float circleRadius,
    const SDL_FRect& rectangle)
{
    const float closestX = std::clamp(
        circleCenter.x,
        rectangle.x,
        rectangle.x + rectangle.w
    );

    const float closestY = std::clamp(
        circleCenter.y,
        rectangle.y,
        rectangle.y + rectangle.h
    );

    const float dx = circleCenter.x - closestX;
    const float dy = circleCenter.y - closestY;

    const float distanceSquared = dx * dx + dy * dy;

    return distanceSquared <= circleRadius * circleRadius;
}
