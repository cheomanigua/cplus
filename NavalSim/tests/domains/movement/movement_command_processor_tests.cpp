
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <vector>

#include "domains/movement/movement_command_processor.hpp"
#include "world/components/tags/entity_type_tags.hpp"

namespace
{

void addCompleteMovementComponents(flecs::entity entity)
{
    entity.set<navalsim::movement::Heading>(navalsim::movement::Heading(90.0));
    entity.set<navalsim::movement::DesiredHeading>(navalsim::movement::DesiredHeading(90.0));
    entity.set<navalsim::movement::TurnRate>(navalsim::movement::TurnRate(2.0));
    entity.set<navalsim::movement::TurnDirection>(navalsim::movement::TurnDirection::None);
    entity.set<navalsim::movement::Speed>(navalsim::movement::Speed(10.0));
    entity.set<navalsim::movement::MaximumSpeed>(navalsim::movement::MaximumSpeed(25.0));
    entity.set<navalsim::movement::MovementStatus>(navalsim::movement::MovementStatus::Underway);
}

flecs::entity createVessel(flecs::world& world)
{
    auto vessel = world.entity();
    vessel.add<navalsim::world::Vessel>();
    addCompleteMovementComponents(vessel);
    return vessel;
}

} // namespace

TEST_CASE("SetDesiredHeadingCommand updates a vessel's desired heading")
{
    flecs::world world;
    auto vessel = createVessel(world);

    navalsim::movement::MovementCommandProcessor processor;
    const std::vector<navalsim::movement::QueuedMovementCommand> commands = {
        {1, navalsim::movement::SetDesiredHeadingCommand{
            vessel.id(),
            navalsim::geography::Bearing(180.0)
        }}
    };

    const auto results = processor.process(world, commands);

    REQUIRE(results.size() == 1);
    CHECK(results[0].commandId == 1);
    CHECK(results[0].status == navalsim::movement::MovementCommandStatus::Applied);
    CHECK(vessel.get<navalsim::movement::DesiredHeading>().degrees() == doctest::Approx(180.0));
}

TEST_CASE("SetSpeedCommand updates a vessel's speed and movement status")
{
    flecs::world world;
    auto vessel = createVessel(world);

    navalsim::movement::MovementCommandProcessor processor;
    const std::vector<navalsim::movement::QueuedMovementCommand> commands = {
        {1, navalsim::movement::SetSpeedCommand{
            vessel.id(),
            navalsim::movement::Speed(15.0)
        }}
    };

    const auto results = processor.process(world, commands);

    REQUIRE(results.size() == 1);
    CHECK(results[0].commandId == 1);
    CHECK(results[0].status == navalsim::movement::MovementCommandStatus::Applied);
    CHECK(vessel.get<navalsim::movement::Speed>().metersPerSecond() == doctest::Approx(15.0));
    CHECK(vessel.get<navalsim::movement::MovementStatus>() == navalsim::movement::MovementStatus::Underway);
}

TEST_CASE("SetSpeedCommand with zero speed stops a vessel")
{
    flecs::world world;
    auto vessel = createVessel(world);

    navalsim::movement::MovementCommandProcessor processor;
    const std::vector<navalsim::movement::QueuedMovementCommand> commands = {
        {1, navalsim::movement::SetSpeedCommand{
            vessel.id(),
            navalsim::movement::Speed(0.0)
        }}
    };

    const auto results = processor.process(world, commands);

    REQUIRE(results.size() == 1);
    CHECK(results[0].commandId == 1);
    CHECK(results[0].status == navalsim::movement::MovementCommandStatus::Applied);
    CHECK(vessel.get<navalsim::movement::Speed>().metersPerSecond() == doctest::Approx(0.0));
    CHECK(vessel.get<navalsim::movement::MovementStatus>() == navalsim::movement::MovementStatus::Stopped);
}

TEST_CASE("SetSpeedCommand accepts a speed equal to the vessel's maximum speed")
{
    flecs::world world;
    auto vessel = createVessel(world);

    navalsim::movement::MovementCommandProcessor processor;
    const std::vector<navalsim::movement::QueuedMovementCommand> commands = {
        {1, navalsim::movement::SetSpeedCommand{
            vessel.id(),
            navalsim::movement::Speed(25.0)
        }}
    };

    const auto results = processor.process(world, commands);

    REQUIRE(results.size() == 1);
    CHECK(results[0].status == navalsim::movement::MovementCommandStatus::Applied);
    CHECK(vessel.get<navalsim::movement::Speed>().metersPerSecond() == doctest::Approx(25.0));
    CHECK(vessel.get<navalsim::movement::MovementStatus>() == navalsim::movement::MovementStatus::Underway);
}

TEST_CASE("SetSpeedCommand rejects a speed above the vessel's maximum speed")
{
    flecs::world world;
    auto vessel = createVessel(world);

    navalsim::movement::MovementCommandProcessor processor;
    const std::vector<navalsim::movement::QueuedMovementCommand> commands = {
        {1, navalsim::movement::SetSpeedCommand{
            vessel.id(),
            navalsim::movement::Speed(30.0)
        }}
    };

    const auto results = processor.process(world, commands);

    REQUIRE(results.size() == 1);
    CHECK(results[0].status == navalsim::movement::MovementCommandStatus::MaximumSpeedExceeded);
    CHECK(vessel.get<navalsim::movement::Speed>().metersPerSecond() == doctest::Approx(10.0));
    CHECK(vessel.get<navalsim::movement::MovementStatus>() == navalsim::movement::MovementStatus::Underway);
}

TEST_CASE("StopMovementCommand stops a vessel")
{
    flecs::world world;
    auto vessel = createVessel(world);

    navalsim::movement::MovementCommandProcessor processor;
    const std::vector<navalsim::movement::QueuedMovementCommand> commands = {
        {1, navalsim::movement::StopMovementCommand{vessel.id()}}
    };

    const auto results = processor.process(world, commands);

    REQUIRE(results.size() == 1);
    CHECK(results[0].commandId == 1);
    CHECK(results[0].status == navalsim::movement::MovementCommandStatus::Applied);
    CHECK(vessel.get<navalsim::movement::Speed>().metersPerSecond() == doctest::Approx(0.0));
    CHECK(vessel.get<navalsim::movement::MovementStatus>() == navalsim::movement::MovementStatus::Stopped);
}

TEST_CASE("Commands targeting a deleted entity are rejected")
{
    flecs::world world;
    auto vessel = createVessel(world);
    const flecs::entity_t target = vessel.id();
    vessel.destruct();

    navalsim::movement::MovementCommandProcessor processor;

    const std::vector<navalsim::movement::QueuedMovementCommand> commands = {
        {1, navalsim::movement::SetDesiredHeadingCommand{target, navalsim::geography::Bearing(180.0)}},
        {2, navalsim::movement::SetSpeedCommand{target, navalsim::movement::Speed(15.0)}},
        {3, navalsim::movement::StopMovementCommand{target}}
    };

    const auto results = processor.process(world, commands);

    REQUIRE(results.size() == 3);
    for (std::size_t i = 0; i < results.size(); ++i)
    {
        CHECK(results[i].commandId == i + 1);
        CHECK(results[i].status == navalsim::movement::MovementCommandStatus::InvalidTarget);
    }
}

TEST_CASE("Commands targeting a non-vessel entity are rejected")
{
    flecs::world world;
    auto entity = world.entity();
    addCompleteMovementComponents(entity);

    navalsim::movement::MovementCommandProcessor processor;

    const std::vector<navalsim::movement::QueuedMovementCommand> commands = {
        {1, navalsim::movement::SetDesiredHeadingCommand{entity.id(), navalsim::geography::Bearing(180.0)}},
        {2, navalsim::movement::SetSpeedCommand{entity.id(), navalsim::movement::Speed(15.0)}},
        {3, navalsim::movement::StopMovementCommand{entity.id()}}
    };

    const auto results = processor.process(world, commands);

    REQUIRE(results.size() == 3);
    for (std::size_t i = 0; i < results.size(); ++i)
    {
        CHECK(results[i].commandId == i + 1);
        CHECK(results[i].status == navalsim::movement::MovementCommandStatus::NotAVessel);
    }
}

TEST_CASE("SetDesiredHeadingCommand rejects vessels missing required components")
{
    flecs::world world;
    auto vessel = world.entity();
    vessel.add<navalsim::world::Vessel>();

    navalsim::movement::MovementCommandProcessor processor;
    const std::vector<navalsim::movement::QueuedMovementCommand> commands = {
        {1, navalsim::movement::SetDesiredHeadingCommand{
            vessel.id(),
            navalsim::geography::Bearing(180.0)
        }}
    };

    const auto results = processor.process(world, commands);

    REQUIRE(results.size() == 1);
    CHECK(results[0].commandId == 1);
    CHECK(results[0].status == navalsim::movement::MovementCommandStatus::MissingRequiredComponent);
}

TEST_CASE("SetSpeedCommand rejects vessels missing required components")
{
    flecs::world world;
    auto vessel = world.entity();
    vessel.add<navalsim::world::Vessel>();

    navalsim::movement::MovementCommandProcessor processor;
    const std::vector<navalsim::movement::QueuedMovementCommand> commands = {
        {1, navalsim::movement::SetSpeedCommand{
            vessel.id(),
            navalsim::movement::Speed(15.0)
        }}
    };

    const auto results = processor.process(world, commands);

    REQUIRE(results.size() == 1);
    CHECK(results[0].commandId == 1);
    CHECK(results[0].status == navalsim::movement::MovementCommandStatus::MissingRequiredComponent);
}

TEST_CASE("SetSpeedCommand rejects vessels missing the maximum speed component")
{
    flecs::world world;
    auto vessel = createVessel(world);
    vessel.remove<navalsim::movement::MaximumSpeed>();

    navalsim::movement::MovementCommandProcessor processor;
    const std::vector<navalsim::movement::QueuedMovementCommand> commands = {
        {1, navalsim::movement::SetSpeedCommand{
            vessel.id(),
            navalsim::movement::Speed(15.0)
        }}
    };

    const auto results = processor.process(world, commands);

    REQUIRE(results.size() == 1);
    CHECK(results[0].status == navalsim::movement::MovementCommandStatus::MissingRequiredComponent);
    CHECK(vessel.get<navalsim::movement::Speed>().metersPerSecond() == doctest::Approx(10.0));
    CHECK(vessel.get<navalsim::movement::MovementStatus>() == navalsim::movement::MovementStatus::Underway);
}

TEST_CASE("StopMovementCommand rejects vessels missing required components")
{
    flecs::world world;
    auto vessel = world.entity();
    vessel.add<navalsim::world::Vessel>();

    navalsim::movement::MovementCommandProcessor processor;
    const std::vector<navalsim::movement::QueuedMovementCommand> commands = {
        {1, navalsim::movement::StopMovementCommand{vessel.id()}}
    };

    const auto results = processor.process(world, commands);

    REQUIRE(results.size() == 1);
    CHECK(results[0].commandId == 1);
    CHECK(results[0].status == navalsim::movement::MovementCommandStatus::MissingRequiredComponent);
}

TEST_CASE("SetDesiredHeadingCommand leaves the desired heading unchanged when rejected")
{
    flecs::world world;
    auto vessel = createVessel(world);
    vessel.set<navalsim::movement::DesiredHeading>(navalsim::movement::DesiredHeading(45.0));
    vessel.remove<navalsim::movement::TurnDirection>();

    navalsim::movement::MovementCommandProcessor processor;
    const std::vector<navalsim::movement::QueuedMovementCommand> commands = {
        {1, navalsim::movement::SetDesiredHeadingCommand{
            vessel.id(),
            navalsim::geography::Bearing(180.0)
        }}
    };

    const auto results = processor.process(world, commands);

    REQUIRE(results.size() == 1);
    CHECK(results[0].status == navalsim::movement::MovementCommandStatus::MissingRequiredComponent);
    CHECK(vessel.get<navalsim::movement::DesiredHeading>().degrees() == doctest::Approx(45.0));
}

TEST_CASE("SetSpeedCommand leaves the speed unchanged when rejected")
{
    flecs::world world;
    auto vessel = createVessel(world);
    vessel.remove<navalsim::movement::MovementStatus>();

    navalsim::movement::MovementCommandProcessor processor;
    const std::vector<navalsim::movement::QueuedMovementCommand> commands = {
        {1, navalsim::movement::SetSpeedCommand{
            vessel.id(),
            navalsim::movement::Speed(20.0)
        }}
    };

    const auto results = processor.process(world, commands);

    REQUIRE(results.size() == 1);
    CHECK(results[0].status == navalsim::movement::MovementCommandStatus::MissingRequiredComponent);
    CHECK(vessel.get<navalsim::movement::Speed>().metersPerSecond() == doctest::Approx(10.0));
}

TEST_CASE("StopMovementCommand leaves the speed unchanged when rejected")
{
    flecs::world world;
    auto vessel = createVessel(world);
    vessel.remove<navalsim::movement::MovementStatus>();

    navalsim::movement::MovementCommandProcessor processor;
    const std::vector<navalsim::movement::QueuedMovementCommand> commands = {
        {1, navalsim::movement::StopMovementCommand{vessel.id()}}
    };

    const auto results = processor.process(world, commands);

    REQUIRE(results.size() == 1);
    CHECK(results[0].status == navalsim::movement::MovementCommandStatus::MissingRequiredComponent);
    CHECK(vessel.get<navalsim::movement::Speed>().metersPerSecond() == doctest::Approx(10.0));
}

TEST_CASE("Multiple movement commands are processed in submission order")
{
    flecs::world world;
    auto vessel = createVessel(world);

    navalsim::movement::MovementCommandProcessor processor;
    const std::vector<navalsim::movement::QueuedMovementCommand> commands = {
        {1, navalsim::movement::SetDesiredHeadingCommand{
            vessel.id(),
            navalsim::geography::Bearing(180.0)
        }},
        {2, navalsim::movement::SetSpeedCommand{
            vessel.id(),
            navalsim::movement::Speed(20.0)
        }},
        {3, navalsim::movement::StopMovementCommand{vessel.id()}}
    };

    const auto results = processor.process(world, commands);

    REQUIRE(results.size() == 3);
    CHECK(results[0].commandId == 1);
    CHECK(results[1].commandId == 2);
    CHECK(results[2].commandId == 3);

    for (const auto& result : results)
    {
        CHECK(result.status == navalsim::movement::MovementCommandStatus::Applied);
    }

    CHECK(vessel.get<navalsim::movement::DesiredHeading>().degrees() == doctest::Approx(180.0));
    CHECK(vessel.get<navalsim::movement::Speed>().metersPerSecond() == doctest::Approx(0.0));
    CHECK(vessel.get<navalsim::movement::MovementStatus>() == navalsim::movement::MovementStatus::Stopped);
}

