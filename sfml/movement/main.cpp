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

Direction getMovementDirection()
{
    Direction direction{};

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
        direction.y -= 1.f;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
        direction.y += 1.f;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
        direction.x -= 1.f;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
        direction.x += 1.f;

    const float length = std::sqrt(
        direction.x * direction.x +
        direction.y * direction.y
    );

    if (length > 0.f)
    {
        direction.x /= length;
        direction.y /= length;
    }

    return direction;
}

int main()
{
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Movement");
    window.setFramerateLimit(60);

    sf::CircleShape circle(15.f);
    circle.setFillColor(sf::Color::Green);

    Position position{400.f, 300.f};
    Speed speed{200.f};

    sf::Clock clock;

    while (window.isOpen())
    {
        const float deltaTime = clock.restart().asSeconds();

        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        const Direction direction = getMovementDirection();

        position.x += direction.x * speed.value * deltaTime;
        position.y += direction.y * speed.value * deltaTime;

        circle.setPosition({position.x, position.y});

        window.clear();
        window.draw(circle);
        window.display();
    }
}
