#include "entt.hpp"
#include <iostream>

struct PositionComp {
    float x{}, y{};
};

struct DirectionComp {
    float x{}, y{};
};

struct SpeedComp {
    float speed{};
};

void movementSystem(entt::registry& registry, float deltaTime)
{
    auto view = registry.view<PositionComp, DirectionComp, SpeedComp>();
    for (auto [entity, pos, vel, spd] : view.each())
    {
        pos.x += vel.x * spd.speed * deltaTime;
        pos.y += vel.y * spd.speed * deltaTime;
    }
}

void printPositions(entt::registry& registry)
{
    auto view = registry.view<PositionComp>();

    for (auto [entity, position] : view.each())
    {
        std::cout << "Entity " << entt::to_entity(entity)
                  << ": "
                  << position.x << ", "
                  << position.y << "\n";
    }
    std::cout << "\n";
}

int main()
{
    constexpr std::uint32_t eSize {5};
    entt::registry registry{};
    std::array<entt::entity, eSize> entities{};

    // Create entities 0, 1, 2, 3, 4.
    for(std::size_t i = 0; i < eSize; ++i)
    {
        auto entity = registry.create();
        entities.at(i) = entity;

        registry.emplace<PositionComp>(
            entity,
            static_cast<float>(i),
            static_cast<float>(i)
        );
        registry.emplace<DirectionComp>(entity, 1.0f, 1.0f);
        registry.emplace<SpeedComp>(entity, 100.0f);
    }

    std::cout << "Before destruction:\n";
    printPositions(registry);


    std::cout << "After destruction of Entity 1:\n";
    registry.destroy(entities[1]);
    printPositions(registry);


    std::cout << "After creating a new entity:\n";
    auto newEntity = registry.create();
    registry.emplace<PositionComp>(newEntity, 5.0f, 5.0f);
    registry.emplace<DirectionComp>(newEntity, 1.0f, 1.0f);
    registry.emplace<SpeedComp>(newEntity, 100.0f);
    printPositions(registry);

    std::cout << "After movement:\n";
    constexpr float deltaTime {1.0f / 60.0f};
    for(size_t frame = 0; frame < 60; ++frame) {
        movementSystem(registry, deltaTime);
    }
    printPositions(registry);
}

