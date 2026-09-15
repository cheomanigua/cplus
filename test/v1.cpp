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

    void destroyEntity(size_t id){
        entityIds.erase(entityIds.begin() + id);
        positions.erase(positions.begin() + id);
    }
};

void movementSystem(Registry& registry, float deltaTime)
{
    for (auto i : registry.entityIds)
    {
        registry.positions[i].x += 1.0f * deltaTime;
        registry.positions[i].y += 1.0f * deltaTime;
    }
}

void printPositions(Registry& registry)
{
    for (auto i : registry.entityIds)
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

    std::cout << "VERSION 1\n";
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

