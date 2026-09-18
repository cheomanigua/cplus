#include <iostream>
#include <cstdint>
#include <cassert>
#include "PositionComp.h"
#include "Registry.h"

void printPositions(const Registry& registry)
{
    for (std::size_t i = 0; i < registry.entities.size(); ++i)
    {
        const EntityId id = registry.entities[i];
        const PositionComp& position = registry.positions[i];

        std::cout
            << "Entity " << id << ": "
            << position.position.x << ", "
            << position.position.y << '\n';
    }
}

int main()
{
    Registry registry{};

    // Create entities 0, 1, 2, 3, 4.
    for (int i = 0; i < 5; ++i)
    {
        registry.createEntity(static_cast<float>(i), static_cast<float>(i));
    }

    std::cout << "VERSION 5\n";
    std::cout << "Before destruction:\n";
    printPositions(registry);

    //for (int frame = 0; frame < 60; ++frame) {
    //    movementSystem(registry, 1.0f / 60.f);
    //}

    registry.destroyEntity(1);
    std::cout << "After destruction of Entity 1:\n";
    printPositions(registry);
    
    std::cout << "After creating a new entity:\n";
    registry.createEntity(5.0f, 5.0f);
    printPositions(registry);
}
