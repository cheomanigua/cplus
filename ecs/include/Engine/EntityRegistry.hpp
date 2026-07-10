#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
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
    EntityRegistry(const std::unordered_map<int32_t, ItemData>& items);
    
    int32_t SpawnNPC(const NPCBlueprint& bp);
    
    void RegisterStats(int32_t entityId, const EntityStats& stats);
    EntityStats* GetEntityStats(int32_t entityId);
    Vector2* GetPosition(int32_t entityId);
    const EntityMetadata& GetMetadata(int32_t entityId) const;
    
    int32_t GetActiveCount() const;
    const std::vector<int32_t>& GetActiveEntities() const;

    void SetSelectedEntity(int32_t id) { _selectedEntityId = id; }
    int32_t GetSelectedEntity() const { return _selectedEntityId; }

    void UpdateEntityCell(int32_t entityId, Vector2 pos);
    const std::vector<int32_t>& GetEntitiesInCell(int x, int y) const;

    void RegisterEntity(int32_t entityId, Vector2 initialPos);

    void ClearGrid();

private:
    int32_t _nextId = 1; 
    int32_t _activeCount = 0;
    int32_t _selectedEntityId = -1;
    
    std::vector<int32_t> _activeEntities;
    std::unordered_map<int32_t, EntityStats> _statsMap;
    std::unordered_map<int32_t, Vector2> _positionMap;
    std::unordered_map<int32_t, EntityMetadata> _metadataMap;
    std::unordered_map<int32_t, ItemData> _items;
    std::vector<int32_t> _grid[EngineConfig::GridWidth][EngineConfig::GridHeight];
};
