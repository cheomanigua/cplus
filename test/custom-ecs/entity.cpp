#include <array>
#include <vector>
#include <iostream>
#include <cstdint>
#include <cassert>

#include <raylib.h>

using EntityId = std::uint32_t;

static constexpr std::size_t MAX_ENTITIES { 5'000 };


struct PositionComp
{
    Vector2 position;
};


class EntityManager
{
public:
    EntityManager()
        : m_freeIds{}
    {
        m_freeIds.reserve(MAX_ENTITIES);
    }

    EntityId create()
    {
        // Reuse a previously destroyed ID first.
        if (!m_freeIds.empty())
        {
            EntityId id = m_freeIds.back();
            m_freeIds.pop_back();
            return id;
        }

        // No recycled IDs available.
        assert(m_nextId < MAX_ENTITIES);

        return m_nextId++;
    }

    void destroy(EntityId id)
    {
        assert(id < MAX_ENTITIES);

        m_freeIds.push_back(id);
    }

private:
    EntityId m_nextId{};
    std::vector<EntityId> m_freeIds{};
};


struct Registry
{
    EntityManager entityManager{};

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


    EntityId createEntity(float posX, float posY)
    {
        EntityId id { entityManager.create() };

        const std::size_t index { entities.size() };

        entities.push_back(id);
        positions.push_back({posX, posY});

        // Store where this entity lives in the dense arrays.
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

        // Make the ID available for recycling.
        entityManager.destroy(id);
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

    //std::cout << "Alive entities:\n";
    //printPositions(registry);

    //// Destroy entity 2.
    //registry.destroyEntity(2);
    //std::cout << "\nDestroyed entity 2:\n";
    //printPositions(registry);

    //// ID 2 should be recycled.
    //registry.createEntity(12.0f, 12.0f);
    //std::cout << "\nCreated new entity:\n";
    //printPositions(registry);

    //// ID 5 should be created because all other IDs
    //// are currently alive.
    //registry.createEntity(18.0f, 18.0f);
    //std::cout << "\nCreated new entity:\n";
    //printPositions(registry);




    std::cout << "ENTITY\n";
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
