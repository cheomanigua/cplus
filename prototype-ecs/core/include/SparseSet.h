#pragma once
#include "Types.h"
#include <vector>
#include <algorithm>

class SparseSet
{
public:
    void insert(Entity entity, std::size_t componentIndex)
    {
        if (entity >= m_sparse.size())
        {
            m_sparse.resize(entity + 1, NullEntity);
        }
        
        m_sparse[entity] = m_dense.size();
        m_dense.push_back(entity);
    }

    void remove(Entity entity)
    {
        if (!contains(entity)) return;

        std::size_t rowIndex = m_sparse[entity];
        std::size_t lastEntity = m_dense.back();

        // Swap with the last element in dense array
        m_dense[rowIndex] = lastEntity;
        m_sparse[lastEntity] = rowIndex;

        // Remove the target entity
        m_dense.pop_back();
        m_sparse[entity] = NullEntity;
    }

    bool contains(Entity entity) const
    {
        return entity < m_sparse.size() && m_sparse[entity] != NullEntity;
    }

    std::size_t get(Entity entity) const
    {
        return m_sparse[entity];
    }

private:
    std::vector<Entity> m_sparse{};
    std::vector<Entity> m_dense{};
};
