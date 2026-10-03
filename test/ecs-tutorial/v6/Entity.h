#include <vector>

using EntityId = std::uint32_t;
static constexpr std::size_t MAX_ENTITIES { 5'000 };

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

