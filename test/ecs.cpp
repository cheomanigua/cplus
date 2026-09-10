#include <vector>
#include <iostream>
#include <raylib.h>
#include <raymath.h>

// Component
struct PositionComp {
    Vector2 position{};
};

// Component
struct DirectionComp {
    Vector2 direction{};
};

// Component
struct SpeedComp {
    float speed{};
};

struct Registry {
    std::vector<PositionComp> positions{};
    std::vector<DirectionComp> directions{};
    std::vector<SpeedComp> speeds{};

    size_t create_entity(float posX, float posY, float dirX, float dirY, float spd)
    {
        size_t id = positions.size(); // Entity
    
        positions.push_back({ posX, posY });
        directions.push_back({ dirX, dirY });
        speeds.push_back({ spd });
    
        return id;
    }
};

// System
void movement_system(Registry& registry, float dt)
{
    for (size_t i = 0; i < registry.positions.size(); ++i) {
        Vector2 velocity = Vector2Scale(registry.directions[i].direction, registry.speeds[i].speed * dt);
        registry.positions[i].position = Vector2Add(registry.positions[i].position, velocity);
    }
}

void printPosition(Registry& registry)
{
    for (size_t i = 0; i < registry.positions.size(); ++i) {
        std::cout << registry.positions[i].position.x << ", " << registry.positions[i].position.y << "\n";
    }
}


int main()
{
    Registry registry{};

    registry.create_entity(0.0f, 0.0f, 1.0f, 0.0f, 100.0f);
    registry.create_entity(10.0f, 5.0f, 1.0f, 1.0f, 100.0f);
    registry.create_entity(3.0f, 7.0f, 0.0f, 1.0f, 200.0f);

    std::cout << "Initial Positions:\n";
    printPosition(registry);

    // Simulate one second at 60 FPS
    constexpr float deltaTime = 1.0f / 60.0f;
    for (int frame = 0; frame < 60; ++frame) {
        movement_system(registry, deltaTime);
    }

    std::cout << "Positions after one second:\n";
    printPosition(registry);

}
