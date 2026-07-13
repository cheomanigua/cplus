#include "Engine/EntityRegistry.hpp"
#include "Engine/NPCBlueprint.hpp"

EntityRegistry::EntityRegistry(const std::unordered_map<int32_t, ItemData>& items) 
    : _items(items) 
{
    // Pre-allocate buffers to the maximum supported entity count
    _stats.resize(EngineConfig::MaxEntities);
    _metadata.resize(EngineConfig::MaxEntities);
    _isActive.assign(EngineConfig::MaxEntities, false);
}

int32_t EntityRegistry::SpawnNPC(const NPCBlueprint& bp) {
    int32_t newId = _nextId++;
    
    // Store metadata using the newId as a direct index
    _metadata[newId] = { bp.Name, bp.Class, bp.Race }; 
    
    // Initialize stats
    EntityStats stats;
    stats.IsDirty = true;
    stats.Health = 100.0f;
    stats.Mana = 50.0f;
    stats.Strength = 10.0f;
    stats.Intelligence = 10.0f;
    
    RegisterStats(newId, stats);
    
    return newId;
}

void EntityRegistry::RegisterStats(int32_t entityId, const EntityStats& stats) {
    if (!_isActive[entityId]) {
        _isActive[entityId] = true;
        _activeEntities.push_back(entityId); // Ensure the entity is tracked
        _activeCount++;
    }
    _stats[entityId] = stats;
}

EntityStats& EntityRegistry::GetEntityStats(int32_t entityId) {
    // Direct access replaces map lookup
    return _stats[entityId];
}

const EntityMetadata& EntityRegistry::GetMetadata(int32_t entityId) const {
    return _metadata[entityId];
}
