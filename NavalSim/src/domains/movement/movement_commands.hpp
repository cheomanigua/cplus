#pragma once

#include <cstddef>
#include <cstdint>
#include <variant>

#include <flecs.h>

#include "core/geography/geographic_types.hpp"
#include "domains/movement/movement_components.hpp"

namespace navalsim::movement
{

using MovementCommandId = std::uint64_t;

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

struct QueuedMovementCommand
{
    MovementCommandId id;
    MovementCommand command;
};

enum class MovementCommandStatus
{
    Applied,
    InvalidTarget,
    NotAVessel,
    MissingRequiredComponent
};

struct MovementCommandResult
{
    MovementCommandId commandId;
    MovementCommandStatus status;
};

} // namespace navalsim::movement

