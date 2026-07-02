#include "Core/ConsoleGameView.hpp"

void ConsoleGameView::RenderNPCStats(const std::vector<NPCBlueprint>& npcs, EngineDriver& engine) {
    std::cout << "\n--- Final Computed NPC Stats ---" << std::endl;
    for (const auto& npc : npcs) {
        EntityStats* stats = engine.GetRegistry()->GetEntityStats(npc.EntityId);
        
        std::cout << "---------------------------" << std::endl;
        std::cout << "ID:    " << npc.EntityId << std::endl;
        std::cout << "Name:  " << npc.Name << std::endl;
        std::cout << "Class: " << npc.Class << std::endl;
        std::cout << "Race:  " << npc.Race << std::endl;
        std::cout << "Spawn: (" << npc.SpawnPosition.x << ", " << npc.SpawnPosition.y << ")" << std::endl;
        
        if (stats) {
            std::cout << "Computed Stats:" << std::endl;
            std::cout << "  Strength:     " << stats->Strength << std::endl;
            std::cout << "  Intelligence: " << stats->Intelligence << std::endl;
            std::cout << "  Health:       " << stats->Health << std::endl;
            std::cout << "  Mana:         " << stats->Mana << std::endl;
        }
    }
}

void ConsoleGameView::Render(const CharacterSheetDto& data) {
    std::cout << "--- Character Sheet: " << data.Name << " ---" << std::endl;
    std::cout << "Weapon: " << data.Weapon << std::endl;
    std::cout << "Skill: " << data.Skill << std::endl;
    std::cout << "Health: " << data.Health << " | Mana: " << data.Mana << std::endl;
    std::cout << "Strength: " << data.Strength << " | Intelligence: " << data.Intelligence << std::endl;
}


void ConsoleGameView::DrawMesh(int32_t id, const Vector2& position) {
    // Only prints if you want to verify it's working in the background
    std::cout << "Entity " << id << " requested render, but skipping." << std::endl;
}
