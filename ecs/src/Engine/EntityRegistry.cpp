#include "Engine/EntityRegistry.hpp"
#include "Engine/NPCBlueprint.hpp"
#include "Components/StatsComponent.hpp"

EntityRegistry::EntityRegistry(const std::unordered_map<int32_t, ItemData>& items) 
    : _items(items) 
{
    // Pre-allocate buffers to the maximum supported entity count
    _isActive.assign(EngineConfig::MaxEntities, false);
}

int32_t EntityRegistry::SpawnNPC(const NPCBlueprint& bp) {
    int32_t newId = _nextId++;
    
    _isActive[newId] = true;
    _activeEntities.push_back(newId);
    _activeCount++;

    return newId;
}
