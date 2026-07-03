#include "Core/ConsoleGameView.hpp"
#include <iostream>
#include <iomanip>

void ConsoleGameView::DisplayFullCharacterSheet(EntityRegistry& registry, const DataLoader& loader) {
    const auto& activeEntities = registry.GetActiveEntities();
    
    // Header
    std::cout << "\n" << std::left 
              << std::setw(5)  << "ID" 
              << std::setw(12) << "Class" 
              << std::setw(10) << "Race" 
              << std::setw(8)  << "HP" 
              << std::setw(8)  << "Mana" 
              << std::setw(8)  << "Str" 
              << std::setw(8)  << "Int" << std::endl;
    std::cout << std::string(65, '-') << std::endl;

    // Data rows
    for (int32_t id : activeEntities) {
        EntityStats* stats = registry.GetEntityStats(id);
        
        // Metadata is retrieved via our new accessor
        const auto& meta = registry.GetMetadata(id); 
        
        if (stats != nullptr) {
            std::cout << std::left << std::setw(5)  << id 
                      << std::setw(12) << meta.Class 
                      << std::setw(10) << meta.Race 
                      << std::setw(8)  << (int)stats->Health 
                      << std::setw(8)  << (int)stats->Mana 
                      << std::setw(8)  << (int)stats->Strength 
                      << std::setw(8)  << (int)stats->Intelligence 
                      << std::endl;
        }
    }
}

void ConsoleGameView::Render(const CharacterSheetDto& data) {
    std::cout << "--- Character Sheet: " << data.Name << " ---" << std::endl;
    std::cout << "Health: " << data.Health << " | Mana: " << data.Mana << std::endl;
}

void ConsoleGameView::DrawMesh(int32_t id, const Vector2& position) {
    // Background render notification
    std::cout << "[Render] Entity " << id << " at (" << position.x << ", " << position.y << ")" << std::endl;
}
