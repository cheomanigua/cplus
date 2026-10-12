#include "game.h"
#include "input.h"

#include <cmath>

namespace
{
    constexpr float GAME_SPEED = 100.0f;
}

SDL_FRect Rectangle::GetBounds() const
{
    return {
        position.x,
        position.y,
        size.x,
        size.y
    };
}

void Update(
    Circle& circle,
    Rectangle& rectangle,
    const Input& input,
    float deltaTime)
{
    SDL_FPoint direction{
        static_cast<float>(input.right - input.left),
        static_cast<float>(input.down - input.up)
    };

    // Normalize movement so diagonal movement
    // isn't faster.
    const float length = std::sqrt(
            direction.x * direction.x +
            direction.y * direction.y);

    if (length > 0.0f)
    {
        direction.x /= length;
        direction.y /= length;
    }

    circle.position.x += direction.x * GAME_SPEED * deltaTime;
    circle.position.y += direction.y * GAME_SPEED * deltaTime;

    rectangle.position.x += direction.x * GAME_SPEED * deltaTime;
    rectangle.position.y += direction.y * GAME_SPEED * deltaTime;
}
