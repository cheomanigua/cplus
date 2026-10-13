#include "my_library.h"

int main()
{
    if (!mv::Init("Collision Demo", 800, 450, "/usr/share/fonts/xscreensaver/clacon.ttf"))
    {
        return 1;
    }

    constexpr float speed {200.0f};
    mv::Circle circle1{{400.0f, 215.0f}, 20.0f};
    mv::Rectangle rect1{{250.0f, 200.0f}, {30.0f, 30.0f}};

    SDL_Color BLUE{0, 121, 241, 255};
    SDL_Color RED{230, 41, 55, 255};

    while (mv::IsRunning())
    {
        mv::BeginFrame();
        float dt = mv::GetDeltaTime();
        mv::Input input = mv::GetInput();

        SDL_FPoint direction{
            static_cast<float>(input.right - input.left),
            static_cast<float>(input.down - input.up)
        };

        // Normalize diagonal movement if moving horizontally & vertically
        if (direction.x != 0.0f && direction.y != 0.0f)
        {
            direction.x *= 0.70710678f;
            direction.y *= 0.70710678f;
        }

        rect1.position.x += direction.x * speed * dt;
        rect1.position.y += direction.y * speed * dt;

        // Check collision easily using overloaded mv::CheckCollision
        bool collided = mv::CheckCollision(circle1, rect1);

        // Draw elements (switch color if colliding)
        mv::draw(circle1, collided ? RED : BLUE);
        mv::draw(rect1, RED);

        mv::drawText("Move the square with the cursor keys.", 10.0f, 10.0f);
        mv::drawText(collided ? "Collision: YES" : "Collision: NO", 10.0f, 25.0f);

        mv::EndFrame();
    }

    return 0;
}
