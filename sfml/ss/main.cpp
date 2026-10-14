#include "ss.hpp"

int main()
{
    sf::RenderWindow window(sf::VideoMode({800, 600}), "My Game");
    window.setFramerateLimit(60);
    sf::Clock clock;

    sf::Font font("/usr/share/fonts/xscreensaver/clacon.ttf");
    sf::Text coll(font);
    coll.setPosition({0.0f, 20.0f});

    //text.setString({});
    //text.setFillColor(sf::Color::Red);
    //sf::Font font;
    //if (!font.openFromFile("/usr/share/fonts/xscreensaver/clacon.ttf")) return 1;
    //ss::Text::defaultFont = &font;
    //ss::Text text("Hello SFML 3", 20, {20.0f, 20.0f});
    //ss::Text text1({}, 20, {20.0f, 35.0f});

    //ss::Circle player{{100.f, 100.f}, 20.f};
    sf::Texture texture = ss::loadTexture("raybunny.png");
    ss::Sprite player{{100.f, 100.f}, &texture};
    ss::Sprite orc{{150.f, 150.f}, &texture};
    ss::Rectangle wall{{250.f, 150.f}, {100.f, 40.f}};
    ss::Rectangle wall2{{450.f, 250.f}, {150.f, 80.f}};

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

        bool isColliding = ss::CheckCollision(player.GetBounds(), wall.GetBounds());
        coll.setString(isColliding ? "Collided: YES" : "Collided: NO");

        window.clear(sf::Color::Black);
        window.draw(coll);

        ss::draw(window, player);
        ss::draw(window, orc);
        ss::draw(window, wall);
        ss::draw(window, wall2);

        //ss::draw(window, text);
        //ss::draw(window, text1);

        window.display();
    }
}
