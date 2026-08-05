#pragma once
#include "Types.h"
#include "ComponentPool.h"
#include <vector>
#include <unordered_map>
#include <memory>
#include <typeindex>
#include <cassert>
#include <utility>  // <----------- Add this
#include "raylib.h" // In case we pass a Vector2 into spawnEntity

class Registry
{
public:
    Entity createEntity()
    {
        Entity id{m_nextEntityId++};
        m_entities.push_back(id);
        return id;
    }

    void destroyEntity(Entity entity)
    {
        // 1. Remove components from all pools for this entity
        for (auto& pair : m_pools)
        {
            pair.second->remove(entity);
        }

        // 2. Remove from active entities vector
        auto it = std::find(m_entities.begin(), m_entities.end(), entity);
        if (it != m_entities.end())
        {
            m_entities.erase(it);
        }

        // 3. Push ID back to free list for recycling
        m_freeIds.push_back(entity);
    }

    // Expose active entities publicly for systems to iterate over
    const std::vector<Entity>& getEntities() const
    {
        return m_entities;
    }

    template <typename T, typename... Args>
    T& addComponent(Entity entity, Args&&... args)
    {
        auto& pool{getOrCreatePool<T>()};
        T component{std::forward<Args>(args)...};
        pool.insert(entity, component);
        return pool.get(entity);
    }

    template <typename T>
    bool hasComponent(Entity entity) const
    {
        auto it = m_pools.find(std::type_index{typeid(T)});
        if (it == m_pools.end())
        {
            return false;
        }
        return static_pointer_cast<ComponentPool<T>>(it->second)->has(entity);
    }

    template <typename T>
    T& getComponent(Entity entity)
    {
        auto& pool{getPool<T>()};
        return pool.get(entity);
    }

    template <typename T>
    void removeComponent(Entity entity)
    {
        // Access the pool for type T and call remove
        // (Depending on how your registry stores pools, e.g.:)
        auto& pool = getPool<T>(); 
        pool.remove(entity);
    }

    // ************ NEW BLOCK FOR STEP 15 ************ //
    // Variadic template to spawn an entity with any components you want
    template<typename... Components>
    Entity spawnEntity(Components&&... components) {
        Entity e = createEntity();
        // Fold expression to add each component passed into the function
        (addComponent<std::decay_t<Components>>(e, std::forward<Components>(components)), ...);
        return e;
    }
    
    // Simple wrapper for despawning
    void despawnEntity(Entity e) {
        destroyEntity(e);
    }
    // *************** END NEW BLOCK *************** //

private:
    template <typename T>
    ComponentPool<T>& getOrCreatePool()
    {
        std::type_index typeId{typeid(T)};
        auto it = m_pools.find(typeId);
        if (it == m_pools.end())
        {
            auto pool{std::make_shared<ComponentPool<T>>()};
            m_pools[typeId] = pool;
            return *pool;
        }
        return *std::static_pointer_cast<ComponentPool<T>>(it->second);
    }

    template <typename T>
    ComponentPool<T>& getPool()
    {
        auto it = m_pools.find(std::type_index{typeid(T)});
        assert(it != m_pools.end() && "Component pool does not exist for this type!");
        return *std::static_pointer_cast<ComponentPool<T>>(it->second);
    }

    Entity m_nextEntityId{1};
    std::vector<Entity> m_entities{};
    std::vector<Entity> m_freeIds{};
    std::unordered_map<std::type_index, std::shared_ptr<IComponentPool>> m_pools{};
};
