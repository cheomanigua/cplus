#pragma once

#include <variant>

#include <flecs.h>

#include "core/geography/geographic_types.hpp"
#include "domains/movement/movement_components.hpp"
#include "simulation/commands/command.hpp"

namespace navalsim::movement
{

using MovementCommandId = navalsim::simulation::CommandId;

struct SetDesiredHeadingCommand
{
    flecs::entity_t target;
    geography::Bearing heading;
};

struct SetSpeedCommand
{
    flecs::entity_t target;
    Speed speed;
};

struct StopMovementCommand
{
    flecs::entity_t target;
};

using MovementCommand = std::variant<
    SetDesiredHeadingCommand,
    SetSpeedCommand,
    StopMovementCommand
>;

using QueuedMovementCommand =
    navalsim::simulation::QueuedCommand<MovementCommand>;

enum class MovementCommandStatus
{
    Applied,
    InvalidTarget,
    NotAVessel,
    MissingRequiredComponent,
	MaximumSpeedExceeded
};

using MovementCommandResult =
    navalsim::simulation::CommandResult<MovementCommandStatus>;

} // namespace navalsim::movement

