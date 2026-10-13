#include "game.h"
#include "input.h"

namespace
{
    constexpr float GAME_SPEED {100.0f};
    constexpr float DIAGONAL {0.70710678118f};
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

    // Normalize movement so diagonal movement isn't faster.
    if (direction.x != 0.0f && direction.y != 0.0f)
    {
        direction.x *= DIAGONAL;
        direction.y *= DIAGONAL;
    }

    circle.position.x += direction.x * GAME_SPEED * deltaTime;
    circle.position.y += direction.y * GAME_SPEED * deltaTime;

    rectangle.position.x += direction.x * GAME_SPEED * deltaTime;
    rectangle.position.y += direction.y * GAME_SPEED * deltaTime;
}
