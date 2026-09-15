#include <array>
#include <vector>
#include <iostream>
#include <cstdint>
#include <cassert>

#include <raylib.h>

using EntityId = std::uint16_t;

static constexpr std::size_t MAX_ENTITIES { 5'000 };


struct PositionComp
{
    Vector2 position;
};

struct Registry
{
    // Dense entity storage.
    std::vector<EntityId> entities{};

    // Components are stored at the same dense index as entities.
    std::vector<PositionComp> positions{};

    // Stores the dense storage index for each EntityId.
    std::array<std::size_t, MAX_ENTITIES> entityToIndex{};


    Registry()
    {
        // Reserve the dense component/entity arrays as well.
        entities.reserve(MAX_ENTITIES);
        positions.reserve(MAX_ENTITIES);
    }

    EntityId createEntity(float posX, float posY) {
        // Generates a unique, monotonically increasing ID without storing state in Registry
        static EntityId nextId{0};
        EntityId id = nextId++;
    
        // The new entity's data will live at the next available dense index
        size_t index = entities.size();
    
        // Push data to the back of our packed dense arrays
        entities.push_back(id);
        positions.push_back({posX, posY});
    
        // Update the O(1) lookup table
        entityToIndex[id] = index;
    
        return id;
    }

    void destroyEntity(EntityId id)
    {
        assert(id < MAX_ENTITIES);

        const std::size_t index { entityToIndex[id] };
        const std::size_t last { entities.size() - 1 };

        // If the entity being removed isn't already the last entity,
        // move the last entity into its slot.
        if (index != last)
        {
            const EntityId movedId { entities[last] };

            entities[index] = movedId;
            positions[index] = positions[last];

            // The moved entity now lives at 'index'.
            entityToIndex[movedId] = index;
        }

        // Remove the last element.
        entities.pop_back();
        positions.pop_back();
    }
};


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

    std::cout << "VERSION 4\n";
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
