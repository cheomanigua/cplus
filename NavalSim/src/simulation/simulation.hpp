#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

#include "domains/movement/movement_command_processor.hpp"
#include "simulation/clock/simulation_clock.hpp"
#include "simulation/scheduler/simulation_scheduler.hpp"
#include "world/world.hpp"

class Simulation
{
public:
    using WorldConfiguration = std::function<void(World&)>;

    Simulation();
    ~Simulation();

    Simulation(const Simulation&) = delete;
    Simulation& operator=(const Simulation&) = delete;

    bool initialize(const WorldConfiguration& configureWorld = {});
    void shutdown();

    std::size_t advance(double realDeltaSeconds);
    void setTimeScale(double timeScale);

    double simulationTime() const noexcept;
    double timeScale() const noexcept;

    navalsim::movement::MovementCommandId submitMovementCommand(
        navalsim::movement::MovementCommand command
    );

    std::vector<navalsim::movement::MovementCommandResult>
    takeMovementCommandResults();

private:
    void update(double fixedStepSeconds);

    void processMovementCommandPhase();
    void runSimulationSystemsPhase(double fixedStepSeconds);

    std::unique_ptr<World> world_;

    SimulationClock clock_;
    SimulationScheduler scheduler_;

    navalsim::movement::MovementCommandProcessor movementCommandProcessor_;
    std::vector<navalsim::movement::QueuedMovementCommand> pendingMovementCommands_;
    std::vector<navalsim::movement::MovementCommandResult> movementCommandResults_;

    navalsim::movement::MovementCommandId nextMovementCommandId_ = 1;

    bool initialized_ = false;
};

