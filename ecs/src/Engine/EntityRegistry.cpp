#include "Engine/EntityRegistry.hpp"
#include <algorithm>

EntityRegistry::EntityRegistry(std::vector<ItemData> itemDatabase) 
    : _itemDatabase(std::move(itemDatabase)) {}

ItemData* EntityRegistry::GetItem(int32_t id) {
    // 1. Bitwise Type Check
    if ((id & EntityMasks::TYPE_MASK) != EntityMasks::ITEM_MASK) return nullptr;

    // 2. Bounds Check
    if (id >= 0 && id < static_cast<int32_t>(_itemDatabase.size())) 
        return &_itemDatabase[id];

    return nullptr;
}

EntityStats& EntityRegistry::GetStats(int32_t entityId) {
    return _stats[entityId];
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
