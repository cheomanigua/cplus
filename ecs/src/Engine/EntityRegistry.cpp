#include "Engine/EntityRegistry.hpp"
#include "Engine/NPCBlueprint.hpp"
#include "Components/StatsComponent.hpp"

EntityRegistry::EntityRegistry(const std::unordered_map<int32_t, ItemData>& items) 
    : _items(items) 
{
    // Pre-allocate buffers to the maximum supported entity count
    _metadata.resize(EngineConfig::MaxEntities);
    _isActive.assign(EngineConfig::MaxEntities, false);
}

int32_t EntityRegistry::SpawnNPC(const NPCBlueprint& bp, StatsComponent& statsComp) {
    int32_t newId = _nextId++;
    
    // Store metadata using the newId as a direct index
    _metadata[newId] = { bp.Name, bp.Class, bp.Race }; 
    
    // Initialize stats
    EntityStats stats = statsComp.Data[newId];
    stats.IsDirty = true;
    stats.Health = 100.0f;
    stats.Mana = 50.0f;
    stats.Strength = 10.0f;
    stats.Intelligence = 10.0f;
    stats.Dexterity = 5.0f;
    
    _isActive[newId] = true;
    _activeEntities.push_back(newId);
    _activeCount++;

    return newId;
}

const EntityMetadata& EntityRegistry::GetMetadata(int32_t entityId) const {
    return _metadata[entityId];
}
