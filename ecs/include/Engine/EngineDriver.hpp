#pragma once
#include <memory>
#include <string>
#include <vector>
#include "Core/Commands/GameCommand.hpp"
#include "Core/Commands/CommandQueue.hpp"
#include "Core/Interfaces/IEngineFacade.hpp"
#include "Core/FormulaProcessor.hpp"
#include "Engine/EntityRegistry.hpp"
#include "Components/IdentityComponent.hpp"
#include "Components/StatsComponent.hpp"
#include "Components/MovementComponent.hpp"
#include "Components/PositionComponent.hpp"
#include "Engine/DataLoader.hpp"
#include "Systems/SpatialSystem.hpp"

class EngineDriver {
private:
    EntityRegistry* _registry;
    DataLoader& _dataLoader;
    std::unordered_map<int32_t, ItemData> _items;
    
    // Engine State
    CommandQueue _commandQueue;
    SpatialSystem _spatialSystem;

    // Components
    IdentityComponent& _identityComp;
    StatsComponent& _statsComp;
    MovementComponent& _moveComp;
    PositionComponent& _posComp;
    
    IEngineFacade* _view;
    std::string _dataDirectory;

public:
    EngineDriver(IEngineFacade* view, 
             DataLoader& loader, 
             const std::unordered_map<int32_t, ItemData>& items,
             std::string dataPath, 
             EntityRegistry* registry, 
             IdentityComponent& identityComp, 
             StatsComponent& statsComp, 
             PositionComponent& posComp,
             MovementComponent& moveComp);

    void Tick(float deltaTime);
    void AddCommand(GameCommand cmd);
    
    // Update GetRegistry to return the raw pointer directly
    EntityRegistry* GetRegistry() { return _registry; }
    
    void InitializeEntities(DataLoader& loader);

    CommandQueue& GetCommandQueue() { return _commandQueue; }

    // Components
    IdentityComponent& GetIdentityComponent() { return _identityComp; }
    StatsComponent& GetStatsComponent() { return _statsComp; }
    PositionComponent& GetPositionComponent() { return _posComp; }
    const MovementComponent& GetMovementComponent() const { return _moveComp; }
};
