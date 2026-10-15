#include "wrapper.h"

int main()
{
    if (!wp::Init("Collision Demo", 800, 450, "/usr/share/fonts/xscreensaver/clacon.ttf"))
    {
        return 1;
    }

    constexpr float speed {200.0f};

    SDL_Texture* texture = wp::load("raybunny.png");
    wp::Sprite player{{100.f, 100.f}, texture};
    wp::Sprite orc{{150.f, 150.f}, texture};
    wp::Rectangle wall{{250.f, 150.f}, {100.f, 40.f}};
    wp::Rectangle wall2{{450.f, 250.f}, {150.f, 80.f}};

    SDL_Color RED{230, 41, 55, 255};

    while (wp::IsRunning())
    {
        wp::BeginFrame({0, 0, 0, 255});
        const float dt = wp::GetDeltaTime();
        wp::Input input = wp::GetInput();

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

        player.position.x += direction.x * speed * dt;
        player.position.y += direction.y * speed * dt;

        // Check collision easily using overloaded wp::CheckCollision
        bool collided = wp::CheckCollision(player, wall);

        // Draw elements (switch color if colliding)
        wp::draw(wall, RED);
        wp::draw(wall2, RED);
        wp::draw(orc);
        wp::draw(player);

        wp::drawText("Move the square with the cursor keys.", 10.0f, 10.0f);
        wp::drawText(collided ? "Collision: YES" : "Collision: NO", 10.0f, 25.0f);

        wp::EndFrame();
    }

    return 0;
}
