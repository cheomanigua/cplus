#include <iostream>
#include <unordered_map>
#include "Systems/MovementSystem.hpp"
#include "Core/Commands/CommandQueue.hpp"
#include "Components/StatsComponent.hpp"
#include "Components/MovementComponent.hpp"
#include "Components/PositionComponent.hpp"
#include "Engine/EntityRegistry.hpp"
#include "Engine/DataLoader.hpp"
#include "Tests/TestEntity.hpp"

void TestMovementSystem() {
    std::unordered_map<int32_t, ItemData> itemMap;
    CommandQueue queue;
    StatsComponent statsComp;
    MovementComponent moveComp;
    PositionComponent posComp;
    EntityRegistry registry(itemMap);
    DataLoader loader;
    float deltaTime = 1.0f;

    // 1. Use the helper from Tests/TestEntity.hpp
    auto [testEntity, bp] = SpawnFirstNPC(registry, loader);
    
    // 2. Setup initial state
    posComp.Positions[testEntity] = {0.0f, 0.0f};
    moveComp.IsMoving[testEntity] = true;
    moveComp.TargetPositions[testEntity] = {1000.0f, 0.0f};
    
    // 3. Setup command
    GameCommand moveCmd;
    moveCmd.Type = CommandType::Move;
    moveCmd.EntityId = testEntity;
    moveCmd.Velocity = {10.0f, 0.0f}; 
    moveCmd.Speed = 2.0f;
    moveCmd.MoveParams.TargetPosition = {1000.0f, 0.0f};
    queue.Enqueue(moveCmd);

    // 4. Process
    MovementSystem::ProcessCommands(queue, moveComp);
    MovementSystem::Update(moveComp, posComp, registry.GetActiveEntities(), deltaTime);

    // 5. Verify
    Vector2& actualRef = posComp.Positions[testEntity];

    if (actualRef.x == 2.0f && actualRef.y == 0.0f) {
        std::cout << "[TEST] MovementSystem: Passed" << std::endl;
    } else {
        std::cout << "[TEST] MovementSystem: FAILED! Expected (20, 0), Got (" 
                  << actualRef.x << ", " << actualRef.y << ")" << std::endl;
    }
}
