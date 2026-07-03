#include "Engine/EntityRegistry.hpp"
#include "Engine/NPCBlueprint.hpp"

EntityRegistry::EntityRegistry(const std::vector<ItemData>& items) : _items(items) {}

int32_t EntityRegistry::SpawnNPC(const NPCBlueprint& bp) {
    int32_t newId = _nextId++;
    
    // Store metadata so EngineDriver can retrieve Class/Race later
    _metadataMap[newId] = { bp.Name, bp.Class, bp.Race, bp.SpawnPosition }; 
    
    // Initialize stats
    EntityStats stats;
    stats.IsDirty = true;
    stats.Health = 100.0f;
    stats.Mana = 50.0f;
    stats.Strength = 10.0f;
    stats.Intelligence = 10.0f;
    
    // This is the method that was failing to link
    RegisterStats(newId, stats);
    
    // Initialize position
    _positionMap[newId] = bp.SpawnPosition;
    
    _activeEntities.push_back(newId);
    _activeCount++;
    
    return newId;
}

void EntityRegistry::RegisterStats(int32_t entityId, const EntityStats& stats) {
    _statsMap[entityId] = stats;
}

EntityStats* EntityRegistry::GetEntityStats(int32_t entityId) {
    // 1. We look into the map (the container where stats are stored)
    auto it = _statsMap.find(entityId);
    
    // 2. If the ID exists, return the address of the stats
    if (it != _statsMap.end()) {
        return &it->second;
    }
    
    // 3. If the ID doesn't exist, return nullptr (safe to check in your View)
    return nullptr;
}

Vector2* EntityRegistry::GetPosition(int32_t entityId) {
    if (_positionMap.find(entityId) != _positionMap.end()) {
        return &_positionMap[entityId];
    }
    return nullptr;
}

const EntityMetadata& EntityRegistry::GetMetadata(int32_t entityId) const {
    return _metadataMap.at(entityId);
}

int32_t EntityRegistry::GetActiveCount() const {
    return _activeCount;
}

const std::vector<int32_t>& EntityRegistry::GetActiveEntities() const {
    return _activeEntities;
}
