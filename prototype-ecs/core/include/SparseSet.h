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
