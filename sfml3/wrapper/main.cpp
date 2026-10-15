#include "wrapper.h"

int main()
{
    sf::RenderWindow window(sf::VideoMode({800, 600}), "My Game");
    window.setFramerateLimit(60);
    sf::Clock clock;

    sf::Font font("/usr/share/fonts/xscreensaver/clacon.ttf");
    sf::Text coll(font);
    coll.setPosition({0.0f, 20.0f});

    sf::Texture texture = wp::loadTexture("raybunny.png");
    wp::Sprite player{{100.f, 100.f}, &texture};
    wp::Sprite orc{{150.f, 150.f}, &texture};
    wp::Rectangle wall{{250.f, 150.f}, {100.f, 40.f}};
    wp::Rectangle wall2{{450.f, 250.f}, {150.f, 80.f}};

    while (window.isOpen())
    {
        while (const auto event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        const float dt = clock.restart().asSeconds();

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
            player.position.x += 200.f * dt;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
            player.position.x -= 200.f * dt;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
            player.position.y += 200.f * dt;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
            player.position.y -= 200.f * dt;

        bool isColliding = wp::CheckCollision(player.GetBounds(), wall.GetBounds());
        coll.setString(isColliding ? "Collided: YES" : "Collided: NO");

        window.clear(sf::Color::Black);
        window.draw(coll);

        wp::draw(window, player);
        wp::draw(window, orc);
        wp::draw(window, wall);
        wp::draw(window, wall2);

        window.display();
    }
}
