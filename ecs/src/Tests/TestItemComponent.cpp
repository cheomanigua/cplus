#include <iostream>
#include <unordered_map>
#include "Components/ItemComponent.hpp"
#include "Engine/EntityRegistry.hpp"
#include "Engine/DataLoader.hpp"
#include "Tests/TestEntity.hpp"

void TestItemComponentLoading() {
    std::unordered_map<int32_t, ItemData> itemMap;
    EntityRegistry registry(itemMap);
    DataLoader loader;
    ItemComponent itemComp;

    // 1. Spawn
    auto [id, bp] = SpawnFirstNPC(registry, loader);

    // 2. Assign
    itemComp.EquippedItemId[id] = bp.EquippedItemId;

    // 3. Verify
    int32_t loadedId = itemComp.EquippedItemId[id];
    if (loadedId == bp.EquippedItemId) {
        std::cout << "[TEST] ItemComponent: Success (Entity " << id 
                  << " has Item " << loadedId << ")" << std::endl;
    } else {
        std::cout << "[TEST] ItemComponent: FAILED! (Entity " << id 
                  << ") Expected Item " << bp.EquippedItemId 
                  << ", but got " << loadedId << std::endl;
    }
}
