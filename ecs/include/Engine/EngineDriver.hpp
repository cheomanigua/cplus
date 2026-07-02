#pragma once
#include <memory>
#include <string>
#include <vector>
#include "Core/Commands/GameCommand.hpp"
#include "Core/Commands/CommandQueue.hpp"
#include "Core/Interfaces/IEngineFacade.hpp"
#include "Core/FormulaProcessor.hpp"
#include "Engine/EntityRegistry.hpp"
#include "Engine/MovementBuffers.hpp"
#include "Engine/DataLoader.hpp"

class EngineDriver {
private:
    std::unique_ptr<EntityRegistry> _registry;
    DataLoader& _dataLoader;
    
    // Engine State
    CommandQueue _commandQueue;
    MovementBuffers _movementBuffers;
    
    IEngineFacade* _view;
    std::string _dataDirectory;

public:
    EngineDriver(IEngineFacade* view, DataLoader& loader, std::vector<ItemData> items, std::string dir);
    void Tick(float deltaTime);
    void AddCommand(GameCommand cmd);
    EntityRegistry* GetRegistry() { return _registry.get(); }
};
