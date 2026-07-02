#include "Engine/EntityRegistry.hpp"
#include <algorithm>

EntityRegistry::EntityRegistry(std::vector<ItemData> itemDatabase) 
    : _itemDatabase(std::move(itemDatabase)) {

        // Initialize all positions to 0,0
    _positions.fill({0.0f, 0.0f});
}

ItemData* EntityRegistry::GetItem(int32_t id) {
    // 1. Bitwise Type Check
    if ((id & EntityMasks::TYPE_MASK) != EntityMasks::ITEM_MASK) return nullptr;

    // 2. Bounds Check
    if (id >= 0 && id < static_cast<int32_t>(_itemDatabase.size())) 
        return &_itemDatabase[id];

    return nullptr;
}

// 1. The "Fast Path" (Reference)
// Assumes you know the entity exists. Throws/Crashes if index is invalid.
EntityStats& EntityRegistry::GetStats(int32_t entityId) {
    return _stats.at(entityId); // Use .at() for bounds checking, or [] if speed is critical
}

// 2. The "Safe Path" (Pointer)
// Use this in EngineDriver for command processing.
EntityStats* EntityRegistry::GetEntityStats(int32_t entityId) {
    // Check if the ID exists in your collection. 
    // Assuming you have a way to check existence (e.g., _stats.count(entityId) or bounds check)
    if (entityId < 0 || entityId >= _stats.size()) {
        return nullptr;
    }
    
    return &GetStats(entityId);
}

void EntityRegistry::RegisterStats(int32_t entityId, const EntityStats& data) {
    _stats[entityId] = data;
    _activeEntities[_activeCount++] = entityId;
}

void EntityRegistry::ProcessCombat() {
    for (int i = 0; i < _activeCount; i++) {
        int32_t eid = _activeEntities[i];
        EntityStats& data = _stats[eid];

        if (!data.IsDirty) continue;

        // Logic for combat iteration goes here...
        data.IsDirty = false;
    }
}
