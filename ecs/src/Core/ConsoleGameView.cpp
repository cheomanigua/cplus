#include "Core/ConsoleGameView.hpp"
#include <iostream>

// By defining the methods here using the ConsoleGameView:: prefix, 
// you tell the compiler these belong to the class declared in the header.

void ConsoleGameView::DrawMesh(int32_t id, const Transform2D& transform) {
    // Implementation for drawing (if needed)
    // Example: Raylib specific drawing code would go here
}

bool ConsoleGameView::IsActionPressed(const std::string& actionName) {
    // Logic: Map actionName string to Raylib input checks (e.g., IsKeyPressed)
    return false; 
}

void ConsoleGameView::Render(const CharacterSheetDto& data) {
    std::cout << "--- Character Sheet: " << data.Name << " ---" << std::endl;
    std::cout << "Weapon: " << data.Weapon << std::endl;
    std::cout << "Skill: " << data.Skill << std::endl;
    std::cout << "Health: " << data.Health << " | Mana: " << data.Mana << std::endl;
    std::cout << "Strength: " << data.Strength << " | Intelligence: " << data.Intelligence << std::endl;
}
