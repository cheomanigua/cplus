#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <string>
#include "Model.hpp"
#include "raylib.h"

struct NPCBlueprint;

struct EntityMetadata {
    std::string Name;
    std::string Class;
    std::string Race;
    Vector2 SpawnPosition;
};

class EntityRegistry {
public:
    EntityRegistry(const std::vector<ItemData>& items);
    
    int32_t SpawnNPC(const NPCBlueprint& bp);
    
    void RegisterStats(int32_t entityId, const EntityStats& stats);
    EntityStats* GetEntityStats(int32_t entityId);
    Vector2* GetPosition(int32_t entityId);
    const EntityMetadata& GetMetadata(int32_t entityId) const; // New Accessor
    
    int32_t GetActiveCount() const;
    const std::vector<int32_t>& GetActiveEntities() const;

    void SetSelectedEntity(int32_t id) { _selectedEntityId = id; }
    int32_t GetSelectedEntity() const { return _selectedEntityId; }

private:
    int32_t _nextId = 1; 
    int32_t _activeCount = 0;
    int32_t _selectedEntityId = -1;
    
    std::vector<int32_t> _activeEntities;
    std::unordered_map<int32_t, EntityStats> _statsMap;
    std::unordered_map<int32_t, Vector2> _positionMap;
    std::unordered_map<int32_t, EntityMetadata> _metadataMap; // New Storage
    std::vector<ItemData> _items;
};
