#include <iostream>
#include "Core/Commands/CommandQueue.hpp"
#include "Engine/MovementComponent.hpp"
#include "Systems/MovementSystem.hpp"
#include "Core/Constants.hpp"

// Ensure this matches the declaration in TestRunner.cpp
void TestMovementSystem() {
    std::vector<ItemData> items;
    CommandQueue queue;
    MovementComponent buffers;
    EntityRegistry registry(items);
    float deltaTime = 1.0f;
    int32_t testEntity = 0;

    buffers.Positions[testEntity] = {0.0f, 0.0f};
    
    GameCommand moveCmd;
    moveCmd.Type = CommandType::Move;
    moveCmd.EntityId = testEntity;
    moveCmd.Velocity = {10.0f, 5.0f};
    moveCmd.Speed = 2.0f;
    queue.Enqueue(moveCmd);

    MovementSystem::ProcessCommands(queue, buffers);
    MovementSystem::Update(buffers, deltaTime, registry);

    Vector2 actual = buffers.Positions[testEntity];
    if (actual.x == 20.0f && actual.y == 10.0f) {
        std::cout << "[TEST] MovementSystem: Passed" << std::endl;
    } else {
        std::cout << "[TEST] MovementSystem: FAILED!" << std::endl;
    }
}
