#pragma once
#include <memory>
#include <queue>
#include <string>
#include <vector>
#include "Engine/EntityRegistry.hpp"
#include "Core/Commands/GameCommand.hpp"
#include "Core/Interfaces/IEngineFacade.hpp"

class EngineDriver {
private:
    std::unique_ptr<EntityRegistry> _registry;
    std::queue<GameCommand> _queue;
    IEngineFacade* _view;
    std::string _dataDirectory;

public:
    EngineDriver(IEngineFacade* view, 
                 std::vector<ItemData> itemDatabase, 
                 std::string dataDir);
                 
    void Tick(float deltaTime);
    void AddCommand(GameCommand cmd);
};
