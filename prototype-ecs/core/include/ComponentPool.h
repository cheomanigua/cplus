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

    void remove(Entity entity)
    {
        if (m_sparseSet.contains(entity))
        {
            std::size_t indexToRemove = m_sparseSet.get(entity);
            std::size_t lastIndex = m_components.size() - 1;

            if (indexToRemove != lastIndex)
            {
                // Move the last component into the gap
                m_components[indexToRemove] = std::move(m_components[lastIndex]);
                
                // Find which entity owned the last component and update its index in the sparse set
                // (Assuming your SparseSet has a way to find an entity by its dense index, 
                // or you can update it based on how your SparseSet is structured).
            }

            m_components.pop_back();
            m_sparseSet.remove(entity);
        }
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
