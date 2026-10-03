#include<vector>
#include<iostream>
#include <cstdint>

using EntityId = std::uint16_t;

struct PositionComp {
    float x{}, y{};
};

struct Registry {
    std::vector<EntityId> entities{};
    std::vector<PositionComp> positions{};

    EntityId createEntity(float posX, float posY)
    {
        EntityId id = static_cast<EntityId>(entities.size());
        entities.push_back(id);
        positions.push_back({posX, posY});

        return id;
    }

    void destroyEntity(EntityId id)
    {
        entities.erase(entities.begin() + id);
        positions.erase(positions.begin() + id);
    }
};

void movementSystem(Registry& registry, float deltaTime)
{
    for (std::size_t i = 0; i < registry.entities.size(); ++i)
    {
        registry.positions[i].x += 1.0f * deltaTime;
        registry.positions[i].y += 1.0f * deltaTime;
    }
}

void printPositions(Registry& registry)
{
    for (std::size_t i = 0; i < registry.entities.size(); ++i)
    {
        std::cout << "Entity " << i << ": " <<
            registry.positions[i].x << ", " <<
            registry.positions[i].y << "\n";
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

    std::cout << "VERSION 3\n";
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

