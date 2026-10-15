#include "wrapper.h"

int main()
{
    sf::RenderWindow window(sf::VideoMode({800, 600}), "My Game");
    window.setFramerateLimit(60);
    sf::Clock clock;

    constexpr float speed {200.0f};

    sf::Font font("/usr/share/fonts/xscreensaver/clacon.ttf");
    sf::Text my_text(font);
    //my_text.setPosition({0.0f, 20.0f});

    sf::Texture texture = wp::loadTexture("raybunny.png");
    wp::Sprite player{{100.f, 100.f}, &texture};
    wp::Rectangle wall1{{250.f, 150.f}, {100.f, 40.f}};
    wp::Rectangle wall2{{450.f, 250.f}, {150.f, 80.f}};

    while (window.isOpen())
    {
        while (const auto event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        wp::Input input = wp::GetInput();

        sf::Vector2f direction{
            static_cast<float>(input.right - input.left),
            static_cast<float>(input.down - input.up)
        };

        const float dt = clock.restart().asSeconds();

        // Normalize diagonal movement if moving horizontally & vertically
        if (direction.x != 0.0f && direction.y != 0.0f)
        {
            direction.x *= 0.70710678f;
            direction.y *= 0.70710678f;
        }

        player.position.x += direction.x * speed * dt;
        player.position.y += direction.y * speed * dt;

        bool isColliding = wp::CheckCollision(player.GetBounds(), wall1.GetBounds());

        window.clear(sf::Color::Black);
        window.draw(my_text);

        wp::draw(window, player);
        wp::draw(window, wall1, (isColliding ? wp::Red : wp::White));
        wp::draw(window, wall2);

        my_text.setString(isColliding ? "Collided: YES" : "Collided: NO");

        window.display();
    }
}
