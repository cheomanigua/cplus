#pragma once
#include <vector>
#include <array>
#include <memory>
#include "Core/Constants.hpp"
#include "Core/Model.hpp"

// Assuming ItemData exists in a Model.hpp file
struct ItemData; 
struct EntityStats;

class EntityRegistry {
private:
    std::vector<ItemData> _itemDatabase;
    std::array<EntityStats, EngineConfig::MaxEntities> _stats;
    std::array<int32_t, EngineConfig::MaxEntities> _activeEntities;
    int32_t _activeCount = 0;

public:
    EntityRegistry(std::vector<ItemData> itemDatabase);
    ItemData* GetItem(int32_t id);
    EntityStats& GetStats(int32_t entityId);
    void RegisterStats(int32_t entityId, const EntityStats& data);
    void ProcessCombat();
};
