#pragma once
#include <vector>
#include <array>
#include <memory>
#include "Core/Constants.hpp"
#include "Core/Model.hpp"
#include "raylib.h"

// Assuming ItemData exists in a Model.hpp file
struct ItemData; 
struct EntityStats;

class EntityRegistry {
private:
    std::vector<ItemData> _itemDatabase;
    std::array<EntityStats, EngineConfig::MaxEntities> _stats;
    std::array<int32_t, EngineConfig::MaxEntities> _activeEntities;
    int32_t _activeCount = 0;
    std::array<Vector2, EngineConfig::MaxEntities> _positions;

public:
    EntityRegistry(std::vector<ItemData> itemDatabase);

    int32_t GetActiveCount() const { return _activeCount; }
    const std::array<int32_t, EngineConfig::MaxEntities>& GetActiveEntities() const { 
        return _activeEntities; 
    }

    ItemData* GetItem(int32_t id);
    EntityStats& GetStats(int32_t entityId); // The fast path
    EntityStats* GetEntityStats(int entityId); // The safe path
    void RegisterStats(int32_t entityId, const EntityStats& data);
    void ProcessCombat();

    Vector2* GetPosition(int32_t entityId) {
        if (entityId >= 0 && entityId < EngineConfig::MaxEntities) {
            return &_positions[entityId];
        }
        return nullptr;
    }
};
