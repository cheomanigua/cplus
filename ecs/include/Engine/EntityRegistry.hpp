#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include "Model.hpp"
#include "raylib.h"
#include "Core/Constants.hpp"

struct NPCBlueprint;

struct EntityMetadata {
    std::string Name;
    std::string Class;
    std::string Race;
    Vector2 SpawnPosition;
};

class EntityRegistry {
public:
    // Using explicit constructor to initialize flat buffers
    EntityRegistry(const std::unordered_map<int32_t, ItemData>& items);
    
    int32_t SpawnNPC(const NPCBlueprint& bp);
    
    // Direct access methods using entityId as index
    void RegisterStats(int32_t entityId, const EntityStats& stats);
    EntityStats& GetEntityStats(int32_t entityId); 
    Vector2& GetPosition(int32_t entityId);
    const EntityMetadata& GetMetadata(int32_t entityId) const;
    
    int32_t GetActiveCount() const { return _activeCount; }
    const std::vector<int32_t>& GetActiveEntities() const { return _activeEntities; }

    void SetSelectedEntity(int32_t id) { _selectedEntityId = id; }
    int32_t GetSelectedEntity() const { return _selectedEntityId; }

    // Spatial grid remains for lookup, but uses entity IDs
    void UpdateEntityCell(int32_t entityId, Vector2 pos);
    const std::vector<int32_t>& GetEntitiesInCell(int x, int y) const;

    void RegisterEntity(int32_t entityId, Vector2 initialPos);
    void ClearGrid();

private:
    int32_t _nextId = 1; 
    int32_t _activeCount = 0;
    int32_t _selectedEntityId = -1;
    
    // Flat buffers indexed by EntityID for cache efficiency
    std::vector<int32_t> _activeEntities;
    std::vector<EntityStats> _stats;
    std::vector<Vector2> _positions;
    std::vector<EntityMetadata> _metadata;
    std::vector<bool> _isActive;

    // Items and Grid
    std::unordered_map<int32_t, ItemData> _items;
    std::vector<int32_t> _grid[EngineConfig::GridWidth][EngineConfig::GridHeight];
};
