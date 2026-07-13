#include <iostream>
#include <unordered_map>
#include "Systems/MovementSystem.hpp"
#include "Core/Commands/CommandQueue.hpp"
#include "Core/Constants.hpp"
#include "Engine/StatsComponent.hpp"
#include "Engine/MovementComponent.hpp"
#include "Engine/PositionComponent.hpp"
#include "Engine/EntityRegistry.hpp"

void TestMovementSystem() {
    std::unordered_map<int32_t, ItemData> itemMap;
    CommandQueue queue;
    StatsComponent statsComp;
    MovementComponent moveComp;
    PositionComponent posComp;
    EntityRegistry registry(itemMap);
    float deltaTime = 1.0f;
    int32_t testEntity = 0;
    
    // 1. Manually set position in the component instead of calling RegisterEntity
    posComp.Positions[testEntity] = {0.0f, 0.0f};
    moveComp.IsMoving[testEntity] = true;

    // 2. Register activation status in the registry
    EntityStats stats; // Initialize empty or default stats

    // Set movement intent
    Vector2 targetPos = {1000.0f, 0.0f};
    moveComp.TargetPositions[testEntity] = targetPos;
    
    GameCommand moveCmd;
    moveCmd.Type = CommandType::Move;
    moveCmd.EntityId = testEntity;
    moveCmd.Velocity = {10.0f, 0.0f}; 
    moveCmd.Speed = 2.0f;
    moveCmd.MoveParams.TargetPosition = targetPos;
    queue.Enqueue(moveCmd);

    MovementSystem::ProcessCommands(queue, moveComp);
    MovementSystem::Update(moveComp, posComp, registry.GetActiveEntities(), deltaTime);

    Vector2& actualRef = posComp.Positions[testEntity];

    if (actualRef.x == 20.0f && actualRef.y == 0.0f) { // Use actualRef
        std::cout << "[TEST] MovementSystem: Passed" << std::endl;
    } else {
        // Since actualRef is a valid reference, just print its values directly
        std::cout << "[TEST] MovementSystem: FAILED! Expected (20, 0), Got (" 
                  << actualRef.x << ", " << actualRef.y << ")" << std::endl;
    }
}
