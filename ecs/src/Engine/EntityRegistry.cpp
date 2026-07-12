#include "Engine/EntityRegistry.hpp"
#include "Engine/NPCBlueprint.hpp"

EntityRegistry::EntityRegistry(const std::unordered_map<int32_t, ItemData>& items) 
    : _items(items) 
{
    // Pre-allocate buffers to the maximum supported entity count
    _stats.resize(EngineConfig::MaxEntities);
    _positions.resize(EngineConfig::MaxEntities);
    _metadata.resize(EngineConfig::MaxEntities);
    _isActive.assign(EngineConfig::MaxEntities, false);
}

int32_t EntityRegistry::SpawnNPC(const NPCBlueprint& bp) {
    int32_t newId = _nextId++;
    
    // Store metadata using the newId as a direct index
    _metadata[newId] = { bp.Name, bp.Class, bp.Race, bp.SpawnPosition }; 
    
    // Initialize stats
    EntityStats stats;
    stats.IsDirty = true;
    stats.Health = 100.0f;
    stats.Mana = 50.0f;
    stats.Strength = 10.0f;
    stats.Intelligence = 10.0f;
    
    RegisterStats(newId, stats);
    RegisterEntity(newId, bp.SpawnPosition);
    
    return newId;
}

void EntityRegistry::RegisterStats(int32_t entityId, const EntityStats& stats) {
    _stats[entityId] = stats;
    _isActive[entityId] = true;
}

EntityStats& EntityRegistry::GetEntityStats(int32_t entityId) {
    // Direct access replaces map lookup
    return _stats[entityId];
}

Vector2& EntityRegistry::GetPosition(int32_t entityId) {
    // Direct access replaces map lookup
    return _positions[entityId];
}

void EntityRegistry::RegisterEntity(int32_t entityId, Vector2 initialPos) {
    _positions[entityId] = initialPos;
    _activeEntities.push_back(entityId);
    _activeCount++;
}

void EntityRegistry::UpdateEntityCell(int32_t entityId, Vector2 pos) {
    int x = (int)pos.x / EngineConfig::CellSize;
    int y = (int)pos.y / EngineConfig::CellSize;
    
    if (x >= 0 && x < EngineConfig::GridWidth && y >= 0 && y < EngineConfig::GridHeight) {
        _grid[x][y].push_back(entityId);
    }
}

void EntityRegistry::ClearGrid() {
    for (int x = 0; x < EngineConfig::GridWidth; ++x) {
        for (int y = 0; y < EngineConfig::GridHeight; ++y) {
            _grid[x][y].clear();
        }
    }
}

const std::vector<int32_t>& EntityRegistry::GetEntitiesInCell(int x, int y) const {
    if (x >= 0 && x < EngineConfig::GridWidth && y >= 0 && y < EngineConfig::GridHeight) {
        return _grid[x][y];
    }
    static const std::vector<int32_t> empty;
    return empty;
}

const EntityMetadata& EntityRegistry::GetMetadata(int32_t entityId) const {
    return _metadata[entityId];
}
