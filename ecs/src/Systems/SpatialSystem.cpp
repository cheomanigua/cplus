#include "Systems/SpatialSystem.hpp"
#include <iostream>

void SpatialSystem::Clear() {
    for (int x = 0; x < EngineConfig::GridWidth; ++x) {
        for (int y = 0; y < EngineConfig::GridHeight; ++y) {
            _grid[x][y].clear();
        }
    }
}

void SpatialSystem::Update(EntityRegistry& registry) {
    Clear();
    
    // Iterate only over active entities
    const auto& activeEntities = registry.GetActiveEntities();
    
    for (int32_t id : activeEntities) {
        Vector2 pos = registry.GetPosition(id);
        
        int x = static_cast<int>(pos.x / EngineConfig::CellSize);
        int y = static_cast<int>(pos.y / EngineConfig::CellSize);
        
        if (x >= 0 && x < EngineConfig::GridWidth && y >= 0 && y < EngineConfig::GridHeight) {
            _grid[x][y].push_back(id);
        }
    }
}

const std::vector<int32_t>& SpatialSystem::GetEntitiesInCell(int x, int y) const {
    static const std::vector<int32_t> empty;
    if (x >= 0 && x < EngineConfig::GridWidth && y >= 0 && y < EngineConfig::GridHeight) {
        return _grid[x][y];
    }
    return empty;
}
