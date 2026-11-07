#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <cmath>

#include <doctest/doctest.h>
#include <flecs.h>

#include "core/geography/geographic_calculations.hpp"
#include "core/geography/geographic_types.hpp"
#include "domains/movement/movement_systems.hpp"
#include "domains/movement/movement_components.hpp"
#include "world/components/tags/entity_type_tags.hpp"

using namespace navalsim::geography;
using namespace navalsim::movement;

TEST_CASE("Underway vessel moves according to its heading and speed")
{
    flecs::world world;
    registerMovementSystems(world);

    auto vessel = world.entity();

    vessel.add<navalsim::world::Vessel>();
    vessel.set<GeographicPosition>(GeographicPosition{
        Latitude(0.0),
        Longitude(0.0),
        Elevation(0.0)
    });
    vessel.set<Heading>(Heading(90.0));
    vessel.set<Speed>(Speed(10.0));
    vessel.set<MovementStatus>(MovementStatus::Underway);

    const auto initialPosition =
        vessel.get<GeographicPosition>();

    world.run_pipeline(0, 1.0);

    const auto& finalPosition =
        vessel.get<GeographicPosition>();

    const auto distance =
        horizontalDistance(initialPosition, finalPosition);

    CHECK(std::abs(distance.meters() - 10.0) < 1e-6);
}

TEST_CASE("Stopped vessel does not move")
{
    flecs::world world;
    registerMovementSystems(world);

    auto vessel = world.entity();

    vessel.add<navalsim::world::Vessel>();
    vessel.set<GeographicPosition>(GeographicPosition{
        Latitude(0.0),
        Longitude(0.0),
        Elevation(0.0)
    });
    vessel.set<Heading>(Heading(90.0));
    vessel.set<Speed>(Speed(10.0));
    vessel.set<MovementStatus>(MovementStatus::Stopped);

    const auto initialPosition =
        vessel.get<GeographicPosition>();

    world.run_pipeline(0, 1.0);

    const auto& finalPosition =
        vessel.get<GeographicPosition>();

    CHECK(
        horizontalDistance(initialPosition, finalPosition).meters()
        == doctest::Approx(0.0)
    );
}

TEST_CASE("Movement preserves elevation")
{
    flecs::world world;
    registerMovementSystems(world);

    auto vessel = world.entity();

    vessel.add<navalsim::world::Vessel>();
    vessel.set<GeographicPosition>(GeographicPosition{
        Latitude(0.0),
        Longitude(0.0),
        Elevation(150.0)
    });
    vessel.set<Heading>(Heading(90.0));
    vessel.set<Speed>(Speed(10.0));
    vessel.set<MovementStatus>(MovementStatus::Underway);

    world.run_pipeline(0, 1.0);

    CHECK(
        vessel.get<GeographicPosition>().elevation().meters()
        == doctest::Approx(150.0)
    );
}

TEST_CASE("Movement uses the supplied timestep")
{
    flecs::world world;
    registerMovementSystems(world);

    auto vessel = world.entity();

    vessel.add<navalsim::world::Vessel>();
    vessel.set<GeographicPosition>(GeographicPosition{
        Latitude(0.0),
        Longitude(0.0),
        Elevation(0.0)
    });
    vessel.set<Heading>(Heading(90.0));
    vessel.set<Speed>(Speed(10.0));
    vessel.set<MovementStatus>(MovementStatus::Underway);

    const auto initialPosition =
        vessel.get<GeographicPosition>();

    world.run_pipeline(0, 2.5);

    const auto& finalPosition =
        vessel.get<GeographicPosition>();

    const auto distance =
        horizontalDistance(initialPosition, finalPosition);

    CHECK(std::abs(distance.meters() - 25.0) < 1e-6);
}

// -----------------------------------------------------------------------------
// Desired heading controller tests
// -----------------------------------------------------------------------------

TEST_CASE("Starboard turn approaches desired heading")
{
    flecs::world world;
    registerMovementSystems(world);

    auto vessel = world.entity();

    vessel.add<navalsim::world::Vessel>();
    vessel.set<Heading>(Heading(90.0));
    vessel.set<DesiredHeading>(DesiredHeading(100.0));
    vessel.set<TurnRate>(TurnRate(2.0));
    vessel.set<TurnDirection>(TurnDirection::None);

    world.run_pipeline(0, 1.0);

    CHECK(
        std::abs(vessel.get<Heading>().degrees() - 92.0) < 1e-9
    );

    CHECK(
        vessel.get<TurnDirection>() == TurnDirection::Starboard
    );
}

TEST_CASE("Port turn approaches desired heading")
{
    flecs::world world;
    registerMovementSystems(world);

    auto vessel = world.entity();

    vessel.add<navalsim::world::Vessel>();
    vessel.set<Heading>(Heading(90.0));
    vessel.set<DesiredHeading>(DesiredHeading(80.0));
    vessel.set<TurnRate>(TurnRate(2.0));
    vessel.set<TurnDirection>(TurnDirection::None);

    world.run_pipeline(0, 1.0);

    CHECK(
        std::abs(vessel.get<Heading>().degrees() - 88.0) < 1e-9
    );

    CHECK(
        vessel.get<TurnDirection>() == TurnDirection::Port
    );
}

TEST_CASE("Heading controller wraps across 360 degrees")
{
    flecs::world world;
    registerMovementSystems(world);

    auto vessel = world.entity();

    vessel.add<navalsim::world::Vessel>();
    vessel.set<Heading>(Heading(359.0));
    vessel.set<DesiredHeading>(DesiredHeading(1.0));
    vessel.set<TurnRate>(TurnRate(2.0));
    vessel.set<TurnDirection>(TurnDirection::None);

    world.run_pipeline(0, 1.0);

    CHECK(
        std::abs(vessel.get<Heading>().degrees() - 1.0) < 1e-9
    );

    CHECK(
        vessel.get<TurnDirection>() == TurnDirection::None
    );
}

TEST_CASE("Heading controller leaves heading unchanged when already aligned")
{
    flecs::world world;
    registerMovementSystems(world);

    auto vessel = world.entity();

    vessel.add<navalsim::world::Vessel>();
    vessel.set<Heading>(Heading(135.0));
    vessel.set<DesiredHeading>(DesiredHeading(135.0));
    vessel.set<TurnRate>(TurnRate(5.0));
    vessel.set<TurnDirection>(TurnDirection::None);

    world.run_pipeline(0, 1.0);

    CHECK(
        std::abs(vessel.get<Heading>().degrees() - 135.0) < 1e-9
    );

    CHECK(
        vessel.get<TurnDirection>() == TurnDirection::None
    );
}

TEST_CASE("Zero turn rate leaves heading unchanged")
{
    flecs::world world;
    registerMovementSystems(world);

    auto vessel = world.entity();

    vessel.add<navalsim::world::Vessel>();
    vessel.set<Heading>(Heading(135.0));
    vessel.set<DesiredHeading>(DesiredHeading(145.0));
    vessel.set<TurnRate>(TurnRate(0.0));
    vessel.set<TurnDirection>(TurnDirection::None);

    world.run_pipeline(0, 1.0);

    CHECK(
        std::abs(vessel.get<Heading>().degrees() - 135.0) < 1e-9
    );
}

TEST_CASE("Turning uses the supplied timestep")
{
    flecs::world world;
    registerMovementSystems(world);

    auto vessel = world.entity();

    vessel.add<navalsim::world::Vessel>();
    vessel.set<Heading>(Heading(90.0));
    vessel.set<DesiredHeading>(DesiredHeading(100.0));
    vessel.set<TurnRate>(TurnRate(2.0));
    vessel.set<TurnDirection>(TurnDirection::None);

    world.run_pipeline(0, 2.5);

    CHECK(
        std::abs(vessel.get<Heading>().degrees() - 95.0) < 1e-9
    );
}

// -----------------------------------------------------------------------------
// Heading controller behaviour tests
// -----------------------------------------------------------------------------

TEST_CASE("Heading controller chooses shortest direction")
{
    flecs::world world;
    registerMovementSystems(world);

    auto vessel = world.entity();

    vessel.add<navalsim::world::Vessel>();
    vessel.set<Heading>(Heading(350.0));
    vessel.set<DesiredHeading>(DesiredHeading(10.0));
    vessel.set<TurnRate>(TurnRate(5.0));
    vessel.set<TurnDirection>(TurnDirection::None);

    world.run_pipeline(0, 1.0);

    CHECK(
        std::abs(vessel.get<Heading>().degrees() - 355.0) < 1e-9
    );

    CHECK(
        vessel.get<TurnDirection>() == TurnDirection::Starboard
    );
}

TEST_CASE("Heading controller does not overshoot desired heading")
{
    flecs::world world;
    registerMovementSystems(world);

    auto vessel = world.entity();

    vessel.add<navalsim::world::Vessel>();
    vessel.set<Heading>(Heading(85.0));
    vessel.set<DesiredHeading>(DesiredHeading(90.0));
    vessel.set<TurnRate>(TurnRate(20.0));
    vessel.set<TurnDirection>(TurnDirection::None);

    world.run_pipeline(0, 1.0);

    CHECK(
        std::abs(vessel.get<Heading>().degrees() - 90.0) < 1e-9
    );

    CHECK(
        vessel.get<TurnDirection>() == TurnDirection::None
    );
}

TEST_CASE("Heading controller stops when desired heading is reached")
{
    flecs::world world;
    registerMovementSystems(world);

    auto vessel = world.entity();

    vessel.add<navalsim::world::Vessel>();
    vessel.set<Heading>(Heading(90.0));
    vessel.set<DesiredHeading>(DesiredHeading(90.0));
    vessel.set<TurnRate>(TurnRate(5.0));
    vessel.set<TurnDirection>(TurnDirection::Starboard);

    world.run_pipeline(0, 1.0);

    CHECK(
        std::abs(vessel.get<Heading>().degrees() - 90.0) < 1e-9
    );

    CHECK(
        vessel.get<TurnDirection>() == TurnDirection::None
    );
}

TEST_CASE("Heading controller chooses starboard for a 180 degree turn")
{
    flecs::world world;
    registerMovementSystems(world);

    auto vessel = world.entity();

    vessel.add<navalsim::world::Vessel>();
    vessel.set<Heading>(Heading(0.0));
    vessel.set<DesiredHeading>(DesiredHeading(180.0));
    vessel.set<TurnRate>(TurnRate(10.0));
    vessel.set<TurnDirection>(TurnDirection::None);

    world.run_pipeline(0, 1.0);

    CHECK(
        std::abs(vessel.get<Heading>().degrees() - 10.0) < 1e-9
    );

    CHECK(
        vessel.get<TurnDirection>() == TurnDirection::Starboard
    );
}

// -----------------------------------------------------------------------------
// Turning and movement integration
// -----------------------------------------------------------------------------

TEST_CASE("Turning happens before movement in the same simulation step")
{
    flecs::world world;
    registerMovementSystems(world);

    const GeographicPosition initialPosition{
        Latitude(0.0),
        Longitude(0.0),
        Elevation(0.0)
    };

    auto vessel = world.entity();

    vessel.add<navalsim::world::Vessel>();
    vessel.set<GeographicPosition>(initialPosition);
    vessel.set<Heading>(Heading(0.0));
    vessel.set<DesiredHeading>(DesiredHeading(90.0));
    vessel.set<Speed>(Speed(10.0));
    vessel.set<MovementStatus>(MovementStatus::Underway);
    vessel.set<TurnRate>(TurnRate(90.0));
    vessel.set<TurnDirection>(TurnDirection::None);

    world.run_pipeline(0, 1.0);

    const auto& finalHeading = vessel.get<Heading>();
    const auto& finalPosition = vessel.get<GeographicPosition>();

    CHECK(
        std::abs(finalHeading.degrees() - 90.0) < 1e-9
    );

    const auto distance =
        horizontalDistance(initialPosition, finalPosition);

    CHECK(std::abs(distance.meters() - 10.0) < 1e-6);

    const auto travelBearing =
        initialBearing(initialPosition, finalPosition);

    CHECK(
        std::abs(travelBearing.degrees() - 90.0) < 1e-6
    );
}

