#include "Core/ConsoleGameView.hpp"
#include <iostream>
#include <iomanip>

void ConsoleGameView::DisplayFullCharacterSheet(EntityRegistry& registry, const PositionComponent& posComp, const DataLoader& loader) {
    const auto& activeEntities = registry.GetActiveEntities();
    
    // Header - Added "Name" column
    std::cout << "\n" << std::left 
              << std::setw(5)  << "ID" 
              << std::setw(12) << "Name"  // Added Name
              << std::setw(12) << "Class" 
              << std::setw(10) << "Race" 
              << std::setw(15) << "Pos (X,Y)"
              << std::setw(8)  << "HP" 
              << std::setw(8)  << "Mana" 
              << std::setw(8)  << "Str" 
              << std::setw(8)  << "Int" 
              << std::setw(8)  << "Dex" << std::endl;
    std::cout << std::string(89, '-') << std::endl;

    for (int32_t id : activeEntities) {
        EntityStats& stats = registry.GetEntityStats(id);
        const auto& meta = registry.GetMetadata(id);
        const Vector2& pos = posComp.Positions[id];
        
        std::cout << std::left << std::setw(5)  << id 
                  << std::setw(12) << meta.Name   // Added Name
                  << std::setw(12) << meta.Class 
                  << std::setw(10) << meta.Race 
                  << "(" << (int)pos.x << "," 
                         << (int)pos.y << ")"
                  << std::setw(6) << "" // Spacer for column alignment
                  << std::setw(8)  << (int)stats.Health 
                  << std::setw(8)  << (int)stats.Mana 
                  << std::setw(8)  << (int)stats.Strength 
                  << std::setw(8)  << (int)stats.Intelligence 
                  << std::setw(8)  << (int)stats.Dexterity 
                  << std::endl;
    }
}

void ConsoleGameView::Render(const CharacterSheetDto& data) {
    std::cout << "--- Character Sheet: " << data.Name << " ---" << std::endl;
    std::cout << "Health: " << data.Health << " | Mana: " << data.Mana << std::endl;
}

void ConsoleGameView::DrawMesh(int32_t id, const Vector2& transform, const MovementComponent& moveComp) {
    // Background render notification
    std::cout << "[Render] Entity " << id << " at (" << transform.x << ", " << transform.y << ")" << std::endl;
}

bool ConsoleGameView::IsActionPressed(const std::string& actionName) {
    return false;
}
