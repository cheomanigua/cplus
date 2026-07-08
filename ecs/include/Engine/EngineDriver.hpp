#pragma once
#include <memory>
#include <string>
#include <vector>
#include "Core/Commands/GameCommand.hpp"
#include "Core/Commands/CommandQueue.hpp"
#include "Core/Interfaces/IEngineFacade.hpp"
#include "Core/FormulaProcessor.hpp"
#include "Engine/EntityRegistry.hpp"
#include "Engine/MovementComponent.hpp"
#include "Engine/DataLoader.hpp"

class EngineDriver {
private:
    EntityRegistry* _registry; 
    DataLoader& _dataLoader;
    std::unordered_map<int32_t, ItemData> _items;
    
    // Engine State
    CommandQueue _commandQueue;
    MovementComponent _movementComponent;
    
    IEngineFacade* _view;
    std::string _dataDirectory;

public:
    EngineDriver(IEngineFacade* view, 
             DataLoader& loader, 
             const std::unordered_map<int32_t, ItemData>& items,
             std::string dataPath, 
             EntityRegistry* registry);

    void Tick(float deltaTime);
    void AddCommand(GameCommand cmd);
    
    // Update GetRegistry to return the raw pointer directly
    EntityRegistry* GetRegistry() { return _registry; }
    
    void InitializeEntities(DataLoader& loader);

    CommandQueue& GetCommandQueue() { return _commandQueue; }
};
