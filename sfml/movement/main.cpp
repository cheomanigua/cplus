#include <SFML/Graphics.hpp>
#include <cmath>

struct Circle {
	sf::Vector2f position;
	float radius;
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

	Circle circle{{400.0f, 215.0f}, 20.0f};
    sf::CircleShape draw(circle.radius);
	draw.setOrigin({circle.radius, circle.radius});
	draw.setPosition(circle.position);
    draw.setFillColor(sf::Color::Green);


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
        circle.position += direction * speed * deltaTime;

        draw.setPosition(circle.position);

        window.clear();
        window.draw(draw);
        window.display();
    }
}
