#include <SFML/Graphics.hpp>
#include <cmath>

struct Position
{
    float x{}, y{};
};

struct Direction
{
    float x{}, y{};
};

struct Speed
{
    float value{};
};

int main()
{
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Movement");

    window.setFramerateLimit(60);

    sf::CircleShape circle(15.f);
    circle.setFillColor(sf::Color::Green);

    Position position{400.f, 300.f};
    Direction direction{0.f, 0.f};
    Speed speed{200.f}; // pixels per second

    sf::Clock clock;

    while (window.isOpen())
    {
        const float deltaTime = clock.restart().asSeconds();

        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        // Reset direction every frame.
        direction = {0.f, 0.f};

        // Build direction from WASD.
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
            direction.y -= 1.f;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
            direction.y += 1.f;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
            direction.x -= 1.f;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
            direction.x += 1.f;

        // Normalize direction.
        const float length = std::sqrt(
                direction.x * direction.x +
                direction.y * direction.y
            );

        if (length > 0.f)
        {
            direction.x /= length;
            direction.y /= length;
        }

        // Position = Position + Direction * Speed * DeltaTime
        position.x += direction.x * speed.value * deltaTime;
        position.y += direction.y * speed.value * deltaTime;

        circle.setPosition({position.x, position.y});

        window.clear();
        window.draw(circle);
        window.display();
    }
}
