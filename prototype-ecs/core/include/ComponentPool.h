#pragma once
#include "SparseSet.h"
#include <vector>
#include <memory>

class IComponentPool
{
public:
    virtual ~IComponentPool() = default;
};

template <typename T>
class ComponentPool : public IComponentPool
{
public:
    void insert(Entity entity, T component)
    {
        if (m_sparseSet.contains(entity))
        {
            m_components[m_sparseSet.get(entity)] = component;
            return;
        }

        std::size_t componentIndex{m_components.size()};
        m_components.push_back(component);
        m_sparseSet.insert(entity, componentIndex);
    }

    bool has(Entity entity) const
    {
        return m_sparseSet.contains(entity);
    }

    T& get(Entity entity)
    {
        return m_components[m_sparseSet.get(entity)];
    }

private:
    SparseSet m_sparseSet{};
    std::vector<T> m_components{};
};
