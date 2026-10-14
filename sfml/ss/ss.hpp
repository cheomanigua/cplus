#pragma once

#include <SFML/Graphics.hpp>

#include <algorithm>

namespace ss
{
    // -------------------------------------------------------------
    // Colors
    // -------------------------------------------------------------

    inline const sf::Color Blue{0, 121, 241};
    inline const sf::Color Red{230, 41, 55};
    inline const sf::Color White{255, 255, 255};
    inline const sf::Color Black{0, 0, 0};
    inline const sf::Color Gray{80, 80, 80};

    // -------------------------------------------------------------
    // Game Objects
    // -------------------------------------------------------------

    struct Circle
    {
        sf::Vector2f position{};
        float radius{};
    };

    struct Rectangle
    {
        sf::Vector2f position{};
        sf::Vector2f size{};

        sf::FloatRect GetBounds() const
        {
            return {position, size};
        }
    };

    struct Sprite
    {
        sf::Vector2f position{};
        const sf::Texture* texture{};

        sf::FloatRect GetBounds() const
        {
            if (!texture)
                return {};

            const auto size = texture->getSize();

            return {
                position,
                {
                    static_cast<float>(size.x),
                    static_cast<float>(size.y)
                }
            };
        }
    };


    // -------------------------------------------------------------
    // Textures
    // -------------------------------------------------------------

    inline sf::Texture loadTexture(const char* path)
    {
        sf::Texture texture(path);
        return texture;
    }


    // -------------------------------------------------------------
    // Drawing
    // -------------------------------------------------------------

    inline void draw(
        sf::RenderTarget& target,
        const Circle& circle,
        sf::Color color = Blue)
    {
        sf::CircleShape shape(circle.radius);

        shape.setOrigin({
            circle.radius,
            circle.radius
        });

        shape.setPosition(circle.position);
        shape.setFillColor(sf::Color::Transparent);
        shape.setOutlineColor(color);
        shape.setOutlineThickness(1.f);

        target.draw(shape);
    }

    inline void draw(
        sf::RenderTarget& target,
        const Rectangle& rectangle,
        sf::Color color = Red)
    {
        sf::RectangleShape shape(rectangle.size);

        shape.setPosition(rectangle.position);
        shape.setFillColor(sf::Color::Transparent);
        shape.setOutlineColor(color);
        shape.setOutlineThickness(1.f);

        target.draw(shape);
    }

    inline void draw(
        sf::RenderTarget& target,
        const Sprite& sprite)
    {
        if (!sprite.texture)
            return;

        sf::Sprite drawable(*sprite.texture);
        drawable.setPosition(sprite.position);

        target.draw(drawable);
    }


    // -------------------------------------------------------------
    // Immediate-Style Primitive Drawing
    // -------------------------------------------------------------

    inline void drawCircle(
        sf::RenderTarget& target,
        sf::Vector2f position,
        float radius,
        sf::Color color = Blue)
    {
        draw(target, Circle{position, radius}, color);
    }

    inline void drawRect(
        sf::RenderTarget& target,
        sf::Vector2f position,
        sf::Vector2f size,
        sf::Color color = Red)
    {
        draw(target, Rectangle{position, size}, color);
    }

    // -------------------------------------------------------------
    // Collision Detection
    // -------------------------------------------------------------

    inline bool CheckCollision(
        sf::Vector2f point,
        sf::Vector2f circleCenter,
        float circleRadius)
    {
        const float dx = point.x - circleCenter.x;
        const float dy = point.y - circleCenter.y;

        return dx * dx + dy * dy
            <= circleRadius * circleRadius;
    }

    inline bool CheckCollision(
        sf::Vector2f point,
        const sf::FloatRect& rectangle)
    {
        return rectangle.contains(point);
    }

    inline bool CheckCollision(
        sf::Vector2f centerA,
        float radiusA,
        sf::Vector2f centerB,
        float radiusB)
    {
        const float dx = centerA.x - centerB.x;
        const float dy = centerA.y - centerB.y;
        const float radiusSum = radiusA + radiusB;

        return dx * dx + dy * dy
            <= radiusSum * radiusSum;
    }

    inline bool CheckCollision(
        const sf::FloatRect& a,
        const sf::FloatRect& b)
    {
        return a.findIntersection(b).has_value();
    }

    inline bool CheckCollision(
        sf::Vector2f center,
        float radius,
        const sf::FloatRect& rectangle)
    {
        const float closestX = std::clamp(
            center.x,
            rectangle.position.x,
            rectangle.position.x + rectangle.size.x
        );

        const float closestY = std::clamp(
            center.y,
            rectangle.position.y,
            rectangle.position.y + rectangle.size.y
        );

        const float dx = center.x - closestX;
        const float dy = center.y - closestY;

        return dx * dx + dy * dy <= radius * radius;
    }

    inline bool CheckCollision(
        const Circle& a,
        const Circle& b)
    {
        return CheckCollision(
            a.position, a.radius,
            b.position, b.radius
        );
    }

    inline bool CheckCollision(
        const Rectangle& a,
        const Rectangle& b)
    {
        return CheckCollision(
            a.GetBounds(),
            b.GetBounds()
        );
    }

    inline bool CheckCollision(
        const Circle& circle,
        const Rectangle& rectangle)
    {
        return CheckCollision(
            circle.position,
            circle.radius,
            rectangle.GetBounds()
        );
    }

} // namespace ss
