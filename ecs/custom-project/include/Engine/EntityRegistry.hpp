#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include "Model.hpp"
#include "raylib.h"
#include "Core/Constants.hpp"

struct NPCBlueprint;
struct StatsComponent;

class EntityRegistry {
public:
    // Using explicit constructor to initialize flat buffers
    EntityRegistry(const std::unordered_map<int32_t, ItemData>& items);
    
    int32_t SpawnNPC(const NPCBlueprint& bp);
    
    int32_t GetActiveCount() const { return _activeCount; }
    const std::vector<int32_t>& GetActiveEntities() const { return _activeEntities; }

    void SetSelectedEntity(int32_t id) { _selectedEntityId = id; }
    int32_t GetSelectedEntity() const { return _selectedEntityId; }

private:
    int32_t _nextId = 1; 
    int32_t _activeCount = 0;
    int32_t _selectedEntityId = -1;
    
    // Flat buffers indexed by EntityID for cache efficiency
    std::vector<int32_t> _activeEntities;
    std::vector<bool> _isActive;

    // Items and Grid
    std::unordered_map<int32_t, ItemData> _items;
};
