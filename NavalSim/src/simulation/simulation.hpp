#pragma once

#include <any>
#include <cstddef>
#include <functional>
#include <memory>
#include <stdexcept>
#include <typeinfo>
#include <utility>
#include <vector>

#include "core/geography/geographic_types.hpp"
#include "data/definitions/definition_id.hpp"
#include "domains/movement/movement_command_processor.hpp"
#include "domains/movement/movement_commands.hpp"
#include "simulation/clock/simulation_clock.hpp"
#include "simulation/commands/command.hpp"
#include "simulation/commands/command_dispatcher.hpp"
#include "simulation/commands/command_queue.hpp"
#include "simulation/scheduler/simulation_scheduler.hpp"
#include "world/world.hpp"

namespace navalsim::data {
class DefinitionDatabase;
}

// Legacy movement-command aliases retained for compatibility with the
// original movement-oriented Simulation command API.
namespace navalsim::simulation {
using SimulationCommandId = CommandId;
using SimulationCommand = navalsim::movement::MovementCommand;
using SimulationCommandStatus = navalsim::movement::MovementCommandStatus;
using SimulationCommandResult = CommandResult<SimulationCommandStatus>;
}

class Simulation {
public:
    using WorldConfiguration = std::function<void(World&)>;
    using CommandTask = std::function<std::any(navalsim::simulation::CommandId)>;

    Simulation();
    ~Simulation();

    Simulation(const Simulation&) = delete;
    Simulation& operator=(const Simulation&) = delete;

    bool initialize(const WorldConfiguration& configureWorld = {});
    bool initialize(const navalsim::data::DefinitionDatabase& definitions, const WorldConfiguration& configureWorld = {});

    flecs::entity_t createVessel(
        const navalsim::data::DefinitionId& definitionId,
        navalsim::geography::GeographicPosition position,
        double initialHeadingDegrees = 0.0
    );

    // Destroys the runtime world and clears pending commands, uncollected results
    // and failures. Handler registrations and the command ID sequence are retained.
    // The object can subsequently be initialized again.
    void shutdown();

    std::size_t advance(double realDeltaSeconds);
    void setTimeScale(double timeScale);

    double simulationTime() const noexcept;
    double timeScale() const noexcept;

    // Registers one handler for a concrete command type.
    // The handler must accept (Simulation&, const Command&) and return Status.
    // A command type can only be registered once; duplicate registration throws
    // std::logic_error. Registrations survive shutdown() and reinitialization.
    // Handlers may register other command types while commands are executing.
    template <typename Command, typename Status, typename Callable>
    void registerCommandHandler(Callable&& callable) {
        commandDispatcher_.registerHandler<Simulation, Command, Status>(
            std::forward<Callable>(callable)
        );
    }

    // Queues a domain-independent command for execution at the next fixed step.
    // Returns a command ID unique for this Simulation object's lifetime.
    // Submission requires an initialized simulation and a registered handler.
    // Commands are processed in global FIFO submission order.
    // Command types must be copy-constructible because the queued task is stored
    // in std::function. Status types must be copy-constructible for std::any storage.
    template <typename Command, typename Status>
    navalsim::simulation::CommandId submitCommand(Command command) {
        if (!initialized_) {
            throw std::logic_error("Cannot submit a command to an uninitialized simulation.");
        }

        if (!commandDispatcher_.hasHandler<Command>()) {
            throw std::logic_error("No handler is registered for this command type.");
        }

        return commandQueue_.submit(
            [this, command = std::move(command)](navalsim::simulation::CommandId commandId) -> std::any {
                return std::any(
                    commandDispatcher_.dispatch<Simulation, Command, Status>(
                        *this,
                        commandId,
                        command
                    )
                );
            }
        );
    }

    // Retrieves and removes successful results matching the requested Status.
    // Results of other status types remain available for later retrieval.
    // Results are returned in command execution order within that status type.
    template <typename Status>
    std::vector<navalsim::simulation::CommandResult<Status>> takeCommandResults() {
        using Result = navalsim::simulation::CommandResult<Status>;

        std::vector<Result> results;

        for (auto iterator = commandResults_.begin(); iterator != commandResults_.end();) {
            if (iterator->type() == typeid(Result)) {
                results.push_back(std::any_cast<Result>(std::move(*iterator)));
                iterator = commandResults_.erase(iterator);
            } else {
                ++iterator;
            }
        }

        return results;
    }

    // Retrieves and removes failures from all command types.
    // Each failure retains its command ID, diagnostic message and exception_ptr.
    // Failed commands are not automatically retried.
    std::vector<navalsim::simulation::CommandExecutionFailure> takeCommandFailures();

    // Legacy movement-only API retained for source compatibility.
    // New domains should use the templated registerCommandHandler(),
    // submitCommand<Command, Status>() and takeCommandResults<Status>() API.
    navalsim::simulation::SimulationCommandId submitCommand(
        navalsim::simulation::SimulationCommand command
    );

    std::vector<navalsim::simulation::SimulationCommandResult> takeCommandResults();

    navalsim::movement::MovementCommandId submitMovementCommand(
        navalsim::movement::MovementCommand command
    );

    std::vector<navalsim::movement::MovementCommandResult> takeMovementCommandResults();

private:
    void registerMovementCommandHandlers();
    void update(double fixedStepSeconds);
    void processCommandPhase();
    void runSimulationSystemsPhase(double fixedStepSeconds);

    std::unique_ptr<World> world_;
    const navalsim::data::DefinitionDatabase* definitions_ = nullptr;

    SimulationClock clock_;
    SimulationScheduler scheduler_;

    navalsim::movement::MovementCommandProcessor movementCommandProcessor_;
    navalsim::simulation::CommandDispatcher commandDispatcher_;

    navalsim::simulation::CommandQueue<CommandTask> commandQueue_;
    std::vector<std::any> commandResults_;
    std::vector<navalsim::simulation::CommandExecutionFailure> commandFailures_;

    bool initialized_ = false;
};

