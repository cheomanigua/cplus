#include <array>
#include "Entity.h"

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
