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
    const float length =
        std::sqrt(
            direction.x * direction.x +
            direction.y * direction.y);

    if (length > 0.0f)
    {
        direction.x /= length;
        direction.y /= length;
    }

    const float movement =
        GAME_SPEED * deltaTime;

    const SDL_FPoint delta{
        direction.x * movement,
        direction.y * movement
    };

    circle.position.x += delta.x;
    circle.position.y += delta.y;

    rectangle.position.x += delta.x;
    rectangle.position.y += delta.y;
}
