#include "domains/movement/movement_command_processor.hpp"

#include "world/components/tags/entity_type_tags.hpp"

namespace navalsim::movement
{

namespace
{

template <typename... RequiredComponents>
MovementCommandStatus validateVesselTarget(flecs::entity entity)
{
    if (!entity.is_alive())
    {
        return MovementCommandStatus::InvalidTarget;
    }

    if (!entity.has<navalsim::world::Vessel>())
    {
        return MovementCommandStatus::NotAVessel;
    }

    if (!(entity.has<RequiredComponents>() && ...))
    {
        return MovementCommandStatus::MissingRequiredComponent;
    }

    return MovementCommandStatus::Applied;
}

MovementCommandStatus processCommand(
    flecs::world& world,
    const SetDesiredHeadingCommand& command
)
{
    auto entity = world.entity(command.target);

    const auto validationStatus = validateVesselTarget<
        Heading,
        DesiredHeading,
        TurnRate,
        TurnDirection
    >(entity);

    if (validationStatus != MovementCommandStatus::Applied)
    {
        return validationStatus;
    }

    entity.set<DesiredHeading>(
        DesiredHeading(command.heading.degrees())
    );

    return MovementCommandStatus::Applied;
}

MovementCommandStatus processCommand(
    flecs::world& world,
    const SetSpeedCommand& command
)
{
    auto entity = world.entity(command.target);

    const auto validationStatus = validateVesselTarget<
        Speed,
        MaximumSpeed,
        MovementStatus
    >(entity);

    if (validationStatus != MovementCommandStatus::Applied)
    {
        return validationStatus;
    }

    if (command.speed.metersPerSecond() > entity.get<MaximumSpeed>().metersPerSecond())
    {
        return MovementCommandStatus::MaximumSpeedExceeded;
    }

    entity.set<Speed>(command.speed);

    entity.set<MovementStatus>(
        command.speed.metersPerSecond() > 0.0
            ? MovementStatus::Underway
            : MovementStatus::Stopped
    );

    return MovementCommandStatus::Applied;
}

MovementCommandStatus processCommand(
    flecs::world& world,
    const StopMovementCommand& command
)
{
    auto entity = world.entity(command.target);

    const auto validationStatus = validateVesselTarget<
        Speed,
        MovementStatus
    >(entity);

    if (validationStatus != MovementCommandStatus::Applied)
    {
        return validationStatus;
    }

    entity.set<Speed>(Speed(0.0));
    entity.set<MovementStatus>(MovementStatus::Stopped);

    return MovementCommandStatus::Applied;
}

} // namespace

std::vector<MovementCommandResult>
MovementCommandProcessor::process(
    flecs::world& world,
    const std::vector<QueuedMovementCommand>& commands
) const
{
    std::vector<MovementCommandResult> results;
    results.reserve(commands.size());

    for (const auto& queuedCommand : commands)
    {
        const auto status = std::visit(
            [&world](const auto& command)
            {
                return processCommand(world, command);
            },
            queuedCommand.command
        );

        results.push_back({queuedCommand.id, status});
    }

    return results;
}

} // namespace navalsim::movement

