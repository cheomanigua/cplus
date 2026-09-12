#include<vector>
#include<iostream>

struct PositionComp {
    float x{}, y{};
};

struct Registry {
    std::vector<size_t> entityIds{};
    std::vector<PositionComp> positions{};

    void createEntity(float posX, float posY){
        entityIds.push_back(entityIds.size());
        positions.push_back({posX, posY});
    }
};

void movementSystem(Registry& registry, float deltaTime)
{
    for (size_t id : registry.entityIds)
    {
        registry.positions[id].x += 1.0f;
        registry.positions[id].y += 1.0f;
    }
}

void printPositions(Registry& registry)
{
    for (size_t id : registry.entityIds)
    {
        std::cout << "Entity " << id << ": " <<
            registry.positions[id].x << ", " <<
            registry.positions[id].y << "\n";
    }
}

int main()
{
    Registry registry{};
    registry.createEntity(0.0f, 0.0f);
    printPositions(registry);
    for (int frame = 0; frame < 60; ++frame) {
        movementSystem(registry, 1.0f / 60.f);
    }
    printPositions(registry);
}

