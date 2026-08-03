#pragma once
#include "Types.h"
#include "ComponentPool.h"
#include <vector>
#include <unordered_map>
#include <memory>
#include <typeindex>
#include <cassert>

class Registry
{
public:
    Entity createEntity()
    {
        Entity id{m_nextEntityId++};
        m_entities.push_back(id);
        return id;
    }

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
    std::unordered_map<std::type_index, std::shared_ptr<IComponentPool>> m_pools{};
};
