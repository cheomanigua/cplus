#pragma once
#include <memory>
#include <string>
#include <vector>
#include "Engine/EntityRegistry.hpp"
#include "Core/Commands/GameCommand.hpp"
#include "Core/Commands/CommandQueue.hpp" // New
#include "Engine/MovementBuffers.hpp"     // New
#include "Core/Interfaces/IEngineFacade.hpp"
#include "Engine/DataLoader.hpp"
#include "Core/FormulaProcessor.hpp"

class EngineDriver {
private:
    std::unique_ptr<EntityRegistry> _registry;
    DataLoader& _dataLoader;
    
    // Engine State
    CommandQueue _commandQueue;           // Replaced std::queue
    MovementBuffers _movementBuffers;     // Added
    
    IEngineFacade* _view;
    std::string _dataDirectory;

public:
    EngineDriver(IEngineFacade* view, DataLoader& loader, std::vector<ItemData> items, std::string dir);
    void Tick(float deltaTime);
    void AddCommand(GameCommand cmd);
    EntityRegistry* GetRegistry() { return _registry.get(); }
};
