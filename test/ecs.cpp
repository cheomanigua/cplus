#include <vector>
#include <iostream>
#include <raylib.h>
#include <raymath.h>

struct PositionComp {
    Vector2 position{};
};

struct DirectionComp {
    Vector2 direction{};
};

struct SpeedComp {
    float speed{};
};

struct Registry {
    std::vector<size_t> entityIds{};
    std::vector<PositionComp> positions{};
    std::vector<DirectionComp> directions{};
    std::vector<SpeedComp> speeds{};

    void createEntity(float posX, float posY, float dirX, float dirY, float spd)
    {
        entityIds.push_back(entityIds.size());
        positions.push_back({ posX, posY });
        directions.push_back({ dirX, dirY });
        speeds.push_back({ spd });
    }
};

void movementSystem(Registry& registry, float deltaTime)
{
    for (auto id : registry.entityIds) {
        Vector2 velocity = Vector2Scale(registry.directions[id].direction, registry.speeds[id].speed * deltaTime);
        registry.positions[id].position = Vector2Add(registry.positions[id].position, velocity);
    }
}

void printPositions(Registry& registry)
{
    for (auto id : registry.entityIds) {
        std::cout << "Entity " << id << ": " << 
            registry.positions[id].position.x << ", " << 
            registry.positions[id].position.y << "\n";
    }
}


int main()
{
    Registry registry{};

    registry.createEntity(0.0f, 0.0f, 1.0f, 0.0f, 100.0f);
    registry.createEntity(10.0f, 5.0f, 1.0f, 1.0f, 100.0f);
    registry.createEntity(3.0f, 7.0f, 0.0f, 1.0f, 200.0f);

    std::cout << "Initial Positions:\n";
    printPositions(registry);

    // Simulate one second at 60 FPS
    constexpr float deltaTime = 1.0f / 60.0f;
    for (int frame = 0; frame < 60; ++frame) {
        movementSystem(registry, deltaTime);
    }

    std::cout << "Positions after one second:\n";
    printPositions(registry);

}
