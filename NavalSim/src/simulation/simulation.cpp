
#include "simulation/simulation.hpp"

#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>

#include "domains/movement/movement_systems.hpp"

Simulation::Simulation() = default;

Simulation::~Simulation()
{
    shutdown();
}

bool Simulation::initialize(const WorldConfiguration& configureWorld)
{
    if (initialized_)
    {
        return true;
    }

    // Construct a new world before changing simulation state.
    auto newWorld = std::make_unique<World>();

    // Register simulation domain systems.
    navalsim::movement::registerMovementSystems(newWorld->ecs());

    // Allow systems and other ECS configuration to be registered.
    if (configureWorld)
    {
        configureWorld(*newWorld);
    }

    // Commit the newly configured world.
    world_ = std::move(newWorld);
    initialized_ = true;

    return true;
}

void Simulation::shutdown()
{
    // Destroy runtime world state first.
    world_.reset();

    // Clear commands and results from the previous session.
    pendingMovementCommands_.clear();
    movementCommandResults_.clear();

    // Reset the simulation session.
    clock_.reset();
    scheduler_.reset();

    initialized_ = false;
}

std::size_t Simulation::advance(double realDeltaSeconds)
{
    if (!initialized_)
    {
        throw std::logic_error("Cannot advance an uninitialized simulation.");
    }

    // Validate the clock advance without modifying the clock.
    const double elapsedSimulationSeconds = clock_.previewAdvance(realDeltaSeconds);

    // Validate scheduler capacity before committing either side.
    scheduler_.validateAdvance(elapsedSimulationSeconds);

    // Both validations succeeded. Commit elapsed simulation time.
    clock_.advance(realDeltaSeconds);

    // Execute as many fixed steps as the scheduler can process.
    return scheduler_.advance(elapsedSimulationSeconds, [this](double fixedStepSeconds)
    {
        update(fixedStepSeconds);
    });
}

void Simulation::setTimeScale(double timeScale)
{
    clock_.setTimeScale(timeScale);
}

double Simulation::simulationTime() const noexcept
{
    return clock_.simulationTime();
}

double Simulation::timeScale() const noexcept
{
    return clock_.timeScale();
}

navalsim::movement::MovementCommandId Simulation::submitMovementCommand(
    navalsim::movement::MovementCommand command
)
{
    if (!initialized_)
    {
        throw std::logic_error("Cannot submit a movement command to an uninitialized simulation.");
    }

    // ID zero is reserved to indicate that the ID counter is exhausted.
    if (nextMovementCommandId_ == 0)
    {
        throw std::overflow_error("Movement command ID space is exhausted.");
    }

    const auto commandId = nextMovementCommandId_;

    // Queue the command before advancing the counter. If allocation fails,
    // the ID remains available for the next submission.
    pendingMovementCommands_.push_back({
        commandId,
        std::move(command)
    });

    // Preserve unique IDs, including across shutdown and reinitialization.
    nextMovementCommandId_ =
        commandId == std::numeric_limits<navalsim::movement::MovementCommandId>::max()
            ? 0
            : commandId + 1;

    return commandId;
}

std::vector<navalsim::movement::MovementCommandResult>
Simulation::takeMovementCommandResults()
{
    auto results = std::move(movementCommandResults_);
    movementCommandResults_.clear();

    return results;
}

void Simulation::update(double fixedStepSeconds)
{
    // Phase 1: Process pending movement commands.
    processMovementCommandPhase();

    // Phase 2: Run simulation systems.
    runSimulationSystemsPhase(fixedStepSeconds);
}

void Simulation::processMovementCommandPhase()
{
    if (pendingMovementCommands_.empty())
    {
        return;
    }

    auto results = movementCommandProcessor_.process(
        world_->ecs(),
        pendingMovementCommands_
    );

    movementCommandResults_.insert(
        movementCommandResults_.end(),
        results.begin(),
        results.end()
    );

    pendingMovementCommands_.clear();
}

void Simulation::runSimulationSystemsPhase(double fixedStepSeconds)
{
    // Run Flecs' default pipeline for this fixed simulation step.
    world_->ecs().run_pipeline(0, fixedStepSeconds);
}

