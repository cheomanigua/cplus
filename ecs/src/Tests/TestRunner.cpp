#include "Tests/TestRunner.hpp"
#include <iostream>
#include "Systems/MovementSystem.hpp"
#include "Core/Commands/CommandQueue.hpp"
#include "Engine/MovementComponent.hpp"

// Define individual tests
void TestMovementSystem();

void TestRunner::RunAll() {
    std::cout << "--- Starting Test Suite ---" << std::endl;
    TestMovementSystem();
    // Add new tests here:
    // TestCollision();
    // TestStats();
    std::cout << "--- Test Suite Complete ---" << std::endl;
}
