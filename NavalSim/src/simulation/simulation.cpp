
#include "simulation/simulation.hpp"

#include <exception>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include "data/definitions/definition_database.hpp"
#include "data/definitions/unit_definition.hpp"
#include "domains/movement/movement_components.hpp"
#include "domains/movement/movement_systems.hpp"
#include "world/components/tags/entity_type_tags.hpp"

namespace {

constexpr double metersPerSecondPerKnot = 1852.0 / 3600.0;

template <typename Command>
navalsim::movement::MovementCommandStatus processSingleMovementCommand(
    flecs::world& world,
    navalsim::movement::MovementCommandProcessor& processor,
    const Command& command
) {
    using namespace navalsim::movement;

    std::vector<QueuedMovementCommand> singleCommand;
    singleCommand.push_back({
        1,
        MovementCommand{command}
    });

    const auto results = processor.process(world, singleCommand);

    if (results.size() != 1) {
        throw std::logic_error(
            "Movement command processor returned an unexpected number of results."
        );
    }

    return results.front().status;
}

std::string describeCommandException(const std::exception_ptr& exception) {
    try {
        std::rethrow_exception(exception);
    } catch (const std::exception& error) {
        return error.what();
    } catch (...) {
        return "Command handler threw a non-standard exception.";
    }
}

} // namespace

Simulation::Simulation() {
    registerMovementCommandHandlers();
}

Simulation::~Simulation() {
    shutdown();
}

void Simulation::registerMovementCommandHandlers() {
    using namespace navalsim::movement;

    registerCommandHandler<SetDesiredHeadingCommand, MovementCommandStatus>(
        [this](Simulation&, const SetDesiredHeadingCommand& command) {
            if (!world_) {
                throw std::logic_error("Cannot process a movement command without a world.");
            }

            return processSingleMovementCommand(
                world_->ecs(),
                movementCommandProcessor_,
                command
            );
        }
    );

    registerCommandHandler<SetSpeedCommand, MovementCommandStatus>(
        [this](Simulation&, const SetSpeedCommand& command) {
            if (!world_) {
                throw std::logic_error("Cannot process a movement command without a world.");
            }

            return processSingleMovementCommand(
                world_->ecs(),
                movementCommandProcessor_,
                command
            );
        }
    );

    registerCommandHandler<StopMovementCommand, MovementCommandStatus>(
        [this](Simulation&, const StopMovementCommand& command) {
            if (!world_) {
                throw std::logic_error("Cannot process a movement command without a world.");
            }

            return processSingleMovementCommand(
                world_->ecs(),
                movementCommandProcessor_,
                command
            );
        }
    );
}

bool Simulation::initialize(const WorldConfiguration& configureWorld) {
    if (initialized_) {
        return true;
    }

    auto newWorld = std::make_unique<World>();

    navalsim::movement::registerMovementSystems(newWorld->ecs());

    if (configureWorld) {
        configureWorld(*newWorld);
    }

    world_ = std::move(newWorld);
    initialized_ = true;

    return true;
}

bool Simulation::initialize(
    const navalsim::data::DefinitionDatabase& definitions,
    const WorldConfiguration& configureWorld
) {
    if (initialized_) {
        if (definitions_ != &definitions) {
            throw std::logic_error(
                "Cannot change the definition database while the simulation is initialized."
            );
        }

        return true;
    }

    definitions_ = &definitions;

    try {
        const bool result = initialize(configureWorld);

        if (!result) {
            definitions_ = nullptr;
        }

        return result;
    } catch (...) {
        definitions_ = nullptr;
        throw;
    }
}

flecs::entity_t Simulation::createVessel(
    const navalsim::data::DefinitionId& definitionId,
    navalsim::geography::GeographicPosition position,
    double initialHeadingDegrees
) {
    if (!initialized_ || !world_) {
        throw std::logic_error("Cannot create a vessel in an uninitialized simulation.");
    }

    if (!definitions_) {
        throw std::logic_error("Cannot create a vessel without an initialized definition database.");
    }

    const auto* definition =
        definitions_->find<navalsim::data::UnitDefinition>(definitionId);

    if (!definition) {
        throw std::invalid_argument("Unit definition was not found: " + definitionId.value());
    }

    if (definition->environment() != navalsim::data::UnitEnvironment::Water) {
        throw std::invalid_argument(
            "Cannot create a vessel from a unit definition outside the water environment: "
            + definitionId.value()
        );
    }

    const double maximumSpeedMetersPerSecond =
        definition->maxSpeedKnots() * metersPerSecondPerKnot;

    auto vessel = world_->ecs().entity();

    vessel.add<navalsim::world::Vessel>();
    vessel.set<navalsim::geography::GeographicPosition>(std::move(position));
    vessel.set<navalsim::movement::Heading>(
        navalsim::movement::Heading(initialHeadingDegrees)
    );
    vessel.set<navalsim::movement::DesiredHeading>(
        navalsim::movement::DesiredHeading(initialHeadingDegrees)
    );
    vessel.set<navalsim::movement::Speed>(
        navalsim::movement::Speed(0.0)
    );
    vessel.set<navalsim::movement::MaximumSpeed>(
        navalsim::movement::MaximumSpeed(maximumSpeedMetersPerSecond)
    );
    vessel.set<navalsim::movement::TurnRate>(
        navalsim::movement::TurnRate(0.0)
    );
    vessel.set<navalsim::movement::TurnDirection>(
        navalsim::movement::TurnDirection::None
    );
    vessel.set<navalsim::movement::MovementStatus>(
        navalsim::movement::MovementStatus::Stopped
    );

    return vessel.id();
}

void Simulation::shutdown() {
    world_.reset();
    definitions_ = nullptr;

    commandQueue_.clear();
    commandResults_.clear();
    commandFailures_.clear();

    clock_.reset();
    scheduler_.reset();

    initialized_ = false;
}

std::size_t Simulation::advance(double realDeltaSeconds) {
    if (!initialized_) {
        throw std::logic_error("Cannot advance an uninitialized simulation.");
    }

    const double elapsedSimulationSeconds = clock_.previewAdvance(realDeltaSeconds);

    scheduler_.validateAdvance(elapsedSimulationSeconds);

    clock_.advance(realDeltaSeconds);

    return scheduler_.advance(elapsedSimulationSeconds, [this](double fixedStepSeconds) {
        update(fixedStepSeconds);
    });
}

void Simulation::setTimeScale(double timeScale) {
    clock_.setTimeScale(timeScale);
}

double Simulation::simulationTime() const noexcept {
    return clock_.simulationTime();
}

double Simulation::timeScale() const noexcept {
    return clock_.timeScale();
}

navalsim::simulation::SimulationCommandId Simulation::submitCommand(
    navalsim::simulation::SimulationCommand command
) {
    return submitMovementCommand(std::move(command));
}

std::vector<navalsim::simulation::SimulationCommandResult>
Simulation::takeCommandResults() {
    return takeMovementCommandResults();
}

std::vector<navalsim::simulation::CommandExecutionFailure>
Simulation::takeCommandFailures() {
    auto failures = std::move(commandFailures_);
    commandFailures_.clear();
    return failures;
}

navalsim::movement::MovementCommandId Simulation::submitMovementCommand(
    navalsim::movement::MovementCommand command
) {
    using namespace navalsim::movement;

    return std::visit(
        [this](const auto& concreteCommand) {
            using Command = std::decay_t<decltype(concreteCommand)>;

            return submitCommand<Command, MovementCommandStatus>(concreteCommand);
        },
        command
    );
}

std::vector<navalsim::movement::MovementCommandResult>
Simulation::takeMovementCommandResults() {
    return takeCommandResults<navalsim::movement::MovementCommandStatus>();
}

void Simulation::update(double fixedStepSeconds) {
    processCommandPhase();
    runSimulationSystemsPhase(fixedStepSeconds);
}

void Simulation::processCommandPhase() {
    if (commandQueue_.empty()) {
        return;
    }

    const std::size_t pendingCount = commandQueue_.size();

    if (pendingCount > commandResults_.max_size() - commandResults_.size()
        || pendingCount > commandFailures_.max_size() - commandFailures_.size()) {
        throw std::length_error("Command result storage capacity is exhausted.");
    }

    // Reserve before taking the batch, so a reserve failure leaves the queue intact.
    commandResults_.reserve(commandResults_.size() + pendingCount);
    commandFailures_.reserve(commandFailures_.size() + pendingCount);

    // Taking the batch removes it from the queue. Each command is now attempted
    // once, and a handler failure will not cause earlier commands to be retried.
    auto pendingCommands = commandQueue_.takePending();

    for (const auto& queuedCommand : pendingCommands) {
        try {
            auto result = queuedCommand.command(queuedCommand.id);
            commandResults_.push_back(std::move(result));
        } catch (...) {
            const auto exception = std::current_exception();

            commandFailures_.push_back({
                queuedCommand.id,
                describeCommandException(exception),
                exception
            });
        }
    }
}

void Simulation::runSimulationSystemsPhase(double fixedStepSeconds) {
    world_->ecs().run_pipeline(0, fixedStepSeconds);
}

