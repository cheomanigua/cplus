#pragma once
#include <memory>
#include <queue>
#include <string>
#include <vector>
#include "Engine/EntityRegistry.hpp"
#include "Core/Commands/GameCommand.hpp"
#include "Core/Interfaces/IEngineFacade.hpp"
#include "Engine/DataLoader.hpp"
#include "Core/FormulaProcessor.hpp"

class EngineDriver {
private:
    std::unique_ptr<EntityRegistry> _registry;
    DataLoader& _dataLoader;
    std::queue<GameCommand> _queue;
    IEngineFacade* _view;
    std::string _dataDirectory;

public:
    // Change your constructor declaration to this:
    EngineDriver(IEngineFacade* view, DataLoader& loader, std::vector<ItemData> items, std::string dir);
    void Tick(float deltaTime);
    void AddCommand(GameCommand cmd);
    EntityRegistry* GetRegistry() { return _registry.get(); }
};
