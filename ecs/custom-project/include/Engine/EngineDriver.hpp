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
#include "Components/ItemComponent.hpp"
#include "Components/MovementComponent.hpp"
#include "Components/PositionComponent.hpp"
#include "Engine/DataLoader.hpp"
#include "Systems/SpatialSystem.hpp"

class EngineDriver {
private:
    IEngineFacade* _view;
    EntityRegistry* _registry;
    std::unordered_map<int32_t, ItemData> _items;
    std::string _dataDirectory;
    
    // Engine State
    CommandQueue _commandQueue;
    DataLoader& _dataLoader;
    SpatialSystem _spatialSystem;

    // Components
    IdentityComponent& _identityComp;
    StatsComponent& _statsComp;
    ItemComponent& _itemComp;
    MovementComponent& _moveComp;
    PositionComponent& _posComp;

public:
    EngineDriver(IEngineFacade* view, 
             DataLoader& loader, 
             const std::unordered_map<int32_t, ItemData>& items,
             std::string dataPath, 
             EntityRegistry* registry, 
             IdentityComponent& identityComp, 
             StatsComponent& statsComp, 
             ItemComponent& itemComp, 
             PositionComponent& posComp, 
             MovementComponent& moveComp);

    void Tick(float deltaTime);
    void AddCommand(GameCommand cmd);
    void InitializeEntities(DataLoader& loader);

    /*********************/
    /*****  GETTERS  *****/
    /*********************/

    // Update GetRegistry to return the raw pointer directly
    EntityRegistry* GetRegistry() { return _registry; }

    CommandQueue& GetCommandQueue() { return _commandQueue; }

    // Components Getters
    IdentityComponent& GetIdentityComponent() { return _identityComp; }
    StatsComponent& GetStatsComponent() { return _statsComp; }
    ItemComponent& GetItemComponent() { return _itemComp; }
    PositionComponent& GetPositionComponent() { return _posComp; }
    const MovementComponent& GetMovementComponent() const { return _moveComp; }
};
