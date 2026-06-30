#include "Engine/EngineDriver.hpp"
#include <iostream>

EngineDriver::EngineDriver(IEngineFacade* view, 
                           std::vector<ItemData> itemDatabase, 
                           std::string dataDir)
    : _view(view), _dataDirectory(std::move(dataDir)) {
    
    _registry = std::make_unique<EntityRegistry>(std::move(itemDatabase));
}

void EngineDriver::Tick(float deltaTime) {
    // 1. Process Command Queue
    while (!_queue.empty()) {
        GameCommand cmd = _queue.front();
        _queue.pop();

        switch (cmd.Type) {
            case CommandType::Move:
                // Logic: Access MovementSystem
                break;
            case CommandType::UpdateStats:
                // Logic: Access StatsUpdateSystem
                break;
            case CommandType::EquipItem:
                // Logic: Access EquipmentSystem
                break;
        }
    }

    // 2. Trigger Systems
    // Example: _updateSystem->Update(deltaTime);
    
    // 3. Post-Process
    _registry->ProcessCombat();
}

void EngineDriver::AddCommand(GameCommand cmd) {
    _queue.push(cmd);
}
