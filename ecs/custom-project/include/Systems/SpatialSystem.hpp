#pragma once
#include <vector>
#include "Engine/EntityRegistry.hpp"
#include "Components/PositionComponent.hpp"
#include "Core/Constants.hpp"

class SpatialSystem {
public:
    void Clear();
    void Update(EntityRegistry& registry, const PositionComponent& posComp);
    const std::vector<int32_t>& GetEntitiesInCell(int x, int y) const;

private:
    std::vector<int32_t> _grid[EngineConfig::GridWidth][EngineConfig::GridHeight];
};
