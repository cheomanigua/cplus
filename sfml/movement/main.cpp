#include <SFML/Graphics.hpp>
#include <cmath>

struct Circle {
	sf::Vector2f position;
	float radius;
};

struct Bunny {
    sf::Vector2f position;
};

sf::Vector2f getMovementDirection()
{
	sf::Vector2f direction = {
	    static_cast<float>(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) -
	    static_cast<float>(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)),
	
	    static_cast<float>(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) -
	    static_cast<float>(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
	};

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
    constexpr float speed { 200.0f };

    // Load a sprite to display
    const sf::Texture texture("raybunny.png");
    Bunny bunny1{{200.0f, 215.0f}};
    sf::Sprite bunny01(texture);

	Circle circle1{{400.0f, 215.0f}, 20.0f};
    sf::CircleShape circle01(circle1.radius, 3);
	circle01.setOrigin({circle1.radius, circle1.radius});
    circle01.setRotation(sf::degrees(90.0f));
    circle01.setFillColor(sf::Color::Red);
    circle01.setOutlineColor(sf::Color::Green);
    circle01.setOutlineThickness(1);

	Circle circle2{{600.0f, 215.0f}, 20.0f};
    sf::CircleShape circle02(circle2.radius);
	circle02.setOrigin({circle2.radius, circle2.radius});
    circle02.setFillColor(sf::Color::Green);

    sf::Clock clock;

    while (window.isOpen())
    {
        const float deltaTime = clock.restart().asSeconds();

        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        const sf::Vector2f direction = getMovementDirection();
        bunny1.position += direction * speed * deltaTime;
        circle2.position.x += 0.0f * speed * deltaTime;
        circle2.position.y += 1.0f * 10.0f * deltaTime;

        bunny01.setPosition(bunny1.position);
        circle01.setPosition(circle1.position);
        circle02.setPosition(circle2.position);

        window.clear();
        window.draw(circle01);
        window.draw(circle02);
        window.draw(bunny01);
        window.display();
    }
}
