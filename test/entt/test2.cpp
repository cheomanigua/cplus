#include "entt.hpp"
#include <iostream>

struct PositionComp {
    float x{}, y{};
};

void movementSystem(entt::registry& registry, float deltaTime)
{
    auto view = registry.view<PositionComp>();

    for (auto [entity, position] : view.each())
    {
        position.x += 1.0f * deltaTime;
        position.y += 1.0f * deltaTime;
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
    entt::registry registry{};
    std::vector<entt::entity> entities{};

    // Create entities 0, 1, 2, 3, 4.
    for (int i = 0; i < 5; ++i)
    {
        auto entity = registry.create();
        entities.push_back(entity);

        registry.emplace<PositionComp>(
            entity,
            static_cast<float>(i),
            static_cast<float>(i)
        );
    }

    std::cout << "Before destruction:\n";
    printPositions(registry);

    // Movement example:
    //
    // for (int frame = 0; frame < 60; ++frame) {
    //     movementSystem(registry, 1.0f / 60.f);
    // }

    // Destroy entity 1.
    //
    // Important: this destroys the entity AND removes its PositionComp.
    registry.destroy(entities[1]);

    std::cout << "After destruction of Entity 1:\n";
    printPositions(registry);

    std::cout << "After creating a new entity:\n";

    auto newEntity = registry.create();
    registry.emplace<PositionComp>(
        newEntity,
        5.0f,
        5.0f
    );
    printPositions(registry);

    registry.destroy(entities[3]);
    newEntity = registry.create();
    registry.emplace<PositionComp>(
        newEntity,
        6.0f,
        6.0f
    );
    printPositions(registry);


    newEntity = registry.create();
    registry.emplace<PositionComp>(
        newEntity,
        7.0f,
        7.0f
    );
    printPositions(registry);
}

