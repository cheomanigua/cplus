#include <iostream>
#include <unordered_map>
#include "Core/Commands/CommandQueue.hpp"
#include "Engine/MovementComponent.hpp"
#include "Systems/MovementSystem.hpp"
#include "Core/Constants.hpp"
#include "Engine/EntityRegistry.hpp"

void TestMovementSystem() {
    std::unordered_map<int32_t, ItemData> itemMap;
    CommandQueue queue;
    MovementComponent buffers;
    EntityRegistry registry(itemMap);
    float deltaTime = 1.0f;
    int32_t testEntity = 0;
    
    // Use your new registration method
    registry.RegisterEntity(testEntity, {0.0f, 0.0f});

    // Set movement intent
    Vector2 targetPos = {1000.0f, 0.0f};
    buffers.TargetPositions[testEntity] = targetPos;
    
    GameCommand moveCmd;
    moveCmd.Type = CommandType::Move;
    moveCmd.EntityId = testEntity;
    moveCmd.Velocity = {10.0f, 0.0f}; 
    moveCmd.Speed = 2.0f;
    moveCmd.MoveParams.TargetPosition = targetPos;
    queue.Enqueue(moveCmd);

    MovementSystem::ProcessCommands(queue, buffers);
    MovementSystem::Update(buffers, deltaTime, registry);

    Vector2* actualPtr = registry.GetPosition(testEntity);
    
    if (actualPtr && actualPtr->x == 20.0f && actualPtr->y == 0.0f) {
        std::cout << "[TEST] MovementSystem: Passed" << std::endl;
    } else {
        float gotX = actualPtr ? actualPtr->x : -1.0f;
        float gotY = actualPtr ? actualPtr->y : -1.0f;
        std::cout << "[TEST] MovementSystem: FAILED! Expected (20, 0), Got (" 
                  << gotX << ", " << gotY << ")" << std::endl;
    }
}
