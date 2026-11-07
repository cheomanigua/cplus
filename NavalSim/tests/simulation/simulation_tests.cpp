#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <cmath>
#include <limits>
#include <stdexcept>
#include <vector>

#include <doctest/doctest.h>
#include <flecs.h>

#include "core/geography/geographic_types.hpp"
#include "core/geography/geographic_calculations.hpp"
#include "domains/movement/movement_components.hpp"
#include "simulation/simulation.hpp"
#include "world/components/tags/entity_type_tags.hpp"


TEST_CASE("Simulation starts with zero simulation time")
{
    Simulation simulation;

    CHECK(simulation.simulationTime() == doctest::Approx(0.0));
    CHECK(simulation.timeScale() == doctest::Approx(1.0));
}

TEST_CASE("Simulation initializes successfully")
{
    Simulation simulation;

    CHECK(simulation.initialize());
}

TEST_CASE("Simulation initialization is idempotent")
{
    Simulation simulation;

    REQUIRE(simulation.initialize());

    simulation.advance(0.5);

    CHECK(simulation.initialize());
    CHECK(simulation.simulationTime() == doctest::Approx(0.5));
}


TEST_CASE("Simulation invokes world configuration during initialization")
{
    Simulation simulation;

    int configurationCalls = 0;

    REQUIRE(simulation.initialize(
        [&configurationCalls](World&)
        {
            ++configurationCalls;
        }
    ));

    CHECK(configurationCalls == 1);

    // Initialization is idempotent: the callback must not run again.
    REQUIRE(simulation.initialize(
        [&configurationCalls](World&)
        {
            ++configurationCalls;
        }
    ));

    CHECK(configurationCalls == 1);
}

TEST_CASE("Simulation advance executes configured Flecs systems")
{
    Simulation simulation;

    std::vector<double> receivedDeltaTimes;

    REQUIRE(simulation.initialize(
        [&receivedDeltaTimes](World& world)
        {
            world.ecs().system<>("TestSystem")
                .kind(flecs::OnUpdate)
                .run([&receivedDeltaTimes](flecs::iter& it)
                {
                    receivedDeltaTimes.push_back(it.delta_time());
                });
        }
    ));

    // The scheduler uses a one-second fixed timestep.
    const std::size_t executedSteps = simulation.advance(2.0);

    REQUIRE(executedSteps == 2);
    REQUIRE(receivedDeltaTimes.size() == 2);

    CHECK(receivedDeltaTimes[0] == doctest::Approx(1.0));
    CHECK(receivedDeltaTimes[1] == doctest::Approx(1.0));
}

TEST_CASE("Simulation remains uninitialized if world configuration throws")
{
    Simulation simulation;

    CHECK_THROWS_AS(
        simulation.initialize(
            [](World&)
            {
                throw std::runtime_error(
                    "Test configuration failure."
                );
            }
        ),
        std::runtime_error
    );

    // The failed initialization must not allow simulation updates.
    CHECK_THROWS_AS(
        simulation.advance(1.0),
        std::logic_error
    );

    // A subsequent initialization without a callback should succeed.
    CHECK(simulation.initialize());

    CHECK(simulation.advance(1.0) == 1);
}

TEST_CASE("Simulation cannot advance before initialization")
{
    Simulation simulation;

    CHECK_THROWS_AS(
        simulation.advance(1.0),
        std::logic_error
    );

    CHECK(simulation.simulationTime() == doctest::Approx(0.0));
}

TEST_CASE("Simulation advances its clock")
{
    Simulation simulation;
    REQUIRE(simulation.initialize());

    simulation.advance(0.5);

    CHECK(simulation.simulationTime() == doctest::Approx(0.5));
}

TEST_CASE("Simulation applies time acceleration")
{
    Simulation simulation;
    REQUIRE(simulation.initialize());

    simulation.setTimeScale(5.0);
    simulation.advance(1.0);

    CHECK(simulation.simulationTime() == doctest::Approx(5.0));
}

TEST_CASE("Simulation uses the fixed-step scheduler")
{
    Simulation simulation;
    REQUIRE(simulation.initialize());

    CHECK(simulation.advance(0.5) == 0);
    CHECK(simulation.advance(0.5) == 1);
}

TEST_CASE("Invalid elapsed time does not advance simulation time")
{
    Simulation simulation;
    REQUIRE(simulation.initialize());

    CHECK_THROWS_AS(
        simulation.advance(-1.0),
        std::invalid_argument
    );

    CHECK(simulation.simulationTime() == doctest::Approx(0.0));
}

TEST_CASE("Non-finite elapsed time does not advance simulation time")
{
    Simulation simulation;
    REQUIRE(simulation.initialize());

    CHECK_THROWS_AS(
        simulation.advance(std::numeric_limits<double>::infinity()),
        std::invalid_argument
    );

    CHECK(simulation.simulationTime() == doctest::Approx(0.0));
}

TEST_CASE("Simulation shutdown resets its state")
{
    Simulation simulation;
    REQUIRE(simulation.initialize());

    simulation.setTimeScale(5.0);
    simulation.advance(1.5);

    simulation.shutdown();

    CHECK(simulation.simulationTime() == doctest::Approx(0.0));
    CHECK(simulation.timeScale() == doctest::Approx(1.0));
}

TEST_CASE("Simulation shutdown is idempotent")
{
    Simulation simulation;
    REQUIRE(simulation.initialize());

    simulation.shutdown();
    simulation.shutdown();

    CHECK_THROWS_AS(
        simulation.advance(1.0),
        std::logic_error
    );
}

TEST_CASE("Simulation can be reinitialized after shutdown")
{
    Simulation simulation;
    REQUIRE(simulation.initialize());

    simulation.setTimeScale(5.0);
    simulation.advance(2.0);

    simulation.shutdown();

    REQUIRE(simulation.initialize());

    CHECK(simulation.simulationTime() == doctest::Approx(0.0));
    CHECK(simulation.timeScale() == doctest::Approx(1.0));

    CHECK(simulation.advance(1.0) == 1);
    CHECK(simulation.simulationTime() == doctest::Approx(1.0));
}

TEST_CASE("Simulation can be shut down before initialization")
{
    Simulation simulation;

    CHECK_NOTHROW(simulation.shutdown());
    CHECK_NOTHROW(simulation.shutdown());
}

TEST_CASE("Flecs pipeline executes systems in phase order")
{
    flecs::world world;

    std::vector<int> executionOrder;
    std::vector<double> receivedDeltaTimes;

    world.system<>("PreUpdateSystem")
        .kind(flecs::PreUpdate)
        .run([&](flecs::iter& it)
        {
            executionOrder.push_back(1);
            receivedDeltaTimes.push_back(it.delta_time());
        });

    world.system<>("OnUpdateSystem")
        .kind(flecs::OnUpdate)
        .run([&](flecs::iter& it)
        {
            executionOrder.push_back(2);
            receivedDeltaTimes.push_back(it.delta_time());
        });

    world.system<>("PostUpdateSystem")
        .kind(flecs::PostUpdate)
        .run([&](flecs::iter& it)
        {
            executionOrder.push_back(3);
            receivedDeltaTimes.push_back(it.delta_time());
        });

    world.run_pipeline(0, 1.0);

    REQUIRE(executionOrder.size() == 3);

    CHECK(executionOrder[0] == 1);
    CHECK(executionOrder[1] == 2);
    CHECK(executionOrder[2] == 3);

    REQUIRE(receivedDeltaTimes.size() == 3);

    CHECK(receivedDeltaTimes[0] == doctest::Approx(1.0));
    CHECK(receivedDeltaTimes[1] == doctest::Approx(1.0));
    CHECK(receivedDeltaTimes[2] == doctest::Approx(1.0));
}

TEST_CASE("Simulation automatically registers movement systems")
{
    Simulation simulation;

    flecs::entity vessel;

    const navalsim::geography::GeographicPosition
        initialPosition(
            navalsim::geography::Latitude(0.0),
            navalsim::geography::Longitude(0.0),
            navalsim::geography::Elevation(100.0)
        );

    REQUIRE(simulation.initialize(
        [&vessel, &initialPosition](World& world)
        {
            vessel = world.ecs().entity("TestVessel");

            vessel.add<navalsim::world::Vessel>();
            vessel.set<navalsim::geography::GeographicPosition>(
                initialPosition
            );
            vessel.set<navalsim::movement::Heading>(
                navalsim::movement::Heading(90.0)
            );
            vessel.set<navalsim::movement::Speed>(
                navalsim::movement::Speed(10.0)
            );
            vessel.set<navalsim::movement::MovementStatus>(
                navalsim::movement::MovementStatus::Underway
            );
        }
    ));

    REQUIRE(simulation.advance(1.0) == 1);

    const auto& finalPosition =
        vessel.get<navalsim::geography::GeographicPosition>();

    CHECK(
        navalsim::geography::horizontalDistance(
            initialPosition,
            finalPosition
        ).meters() == doctest::Approx(10.0).epsilon(0.001)
    );

    CHECK(
        finalPosition.elevation().meters()
        == doctest::Approx(100.0)
    );
}

TEST_CASE("Simulation time acceleration affects vessel movement")
{
    Simulation simulation;

    flecs::entity vessel;

    const navalsim::geography::GeographicPosition
        initialPosition(
            navalsim::geography::Latitude(0.0),
            navalsim::geography::Longitude(0.0),
            navalsim::geography::Elevation(100.0)
        );

    REQUIRE(simulation.initialize(
        [&vessel, &initialPosition](World& world)
        {
            vessel = world.ecs().entity("TestVessel");

            vessel.add<navalsim::world::Vessel>();

            vessel.set<navalsim::geography::GeographicPosition>(
                initialPosition
            );

            vessel.set<navalsim::movement::Heading>(
                navalsim::movement::Heading(90.0)
            );

            vessel.set<navalsim::movement::Speed>(
                navalsim::movement::Speed(10.0)
            );

            vessel.set<navalsim::movement::MovementStatus>(
                navalsim::movement::MovementStatus::Underway
            );
        }
    ));

    // Accelerate simulation time to five times real time.
    simulation.setTimeScale(5.0);

    // Advance one real-world second.
    const std::size_t executedSteps = simulation.advance(1.0);

    // Five one-second simulation steps should execute.
    CHECK(executedSteps == 5);

    // Simulation time should have advanced by five seconds.
    CHECK(simulation.simulationTime() == doctest::Approx(5.0));

    const auto& finalPosition =
        vessel.get<navalsim::geography::GeographicPosition>();

    // At 10 m/s for five simulation seconds, the vessel travels 50 metres.
    CHECK(
        navalsim::geography::horizontalDistance(
            initialPosition,
            finalPosition
        ).meters() == doctest::Approx(50.0).epsilon(0.001)
    );

    // Elevation must remain unchanged.
    CHECK(
        finalPosition.elevation().meters()
        == doctest::Approx(100.0)
    );
}

TEST_CASE("Simulation integrates vessel heading control and movement")
{
    using namespace navalsim::geography;
    using namespace navalsim::movement;

    Simulation simulation;
    flecs::entity vessel;

    REQUIRE_NOTHROW(
        simulation.initialize(
            [&vessel](World& world)
            {
                vessel = world.ecs().entity();

                vessel.add<navalsim::world::Vessel>();

                vessel.set<GeographicPosition>(
                    GeographicPosition(
                        Latitude(0.0),
                        Longitude(0.0),
                        Elevation(0.0)
                    )
                );

                vessel.set<Heading>(Heading(0.0));
                vessel.set<DesiredHeading>(DesiredHeading(90.0));
                vessel.set<Speed>(Speed(10.0));
                vessel.set<TurnRate>(TurnRate(90.0));
                vessel.set<TurnDirection>(TurnDirection::None);
                vessel.set<MovementStatus>(MovementStatus::Underway);
            }
        )
    );

    REQUIRE_NOTHROW(simulation.advance(1.0));

	const auto& heading = vessel.get<Heading>();
    const auto& position = vessel.get<GeographicPosition>();
    const auto& turnDirection = vessel.get<TurnDirection>();

    CHECK(std::abs(heading.degrees() - 90.0) < 1e-9);
    CHECK(turnDirection == TurnDirection::None);

    const GeographicPosition startingPosition(
        Latitude(0.0),
        Longitude(0.0),
        Elevation(0.0)
    );

    const Distance travelled = horizontalDistance(
        startingPosition,
        position
    );

    CHECK(std::abs(travelled.meters() - 10.0) < 1e-5);
    CHECK(std::abs(position.elevation().meters()) < 1e-9);
}



namespace
{

flecs::entity createCommandTestVessel(World& world)
{
    auto vessel = world.ecs().entity("CommandTestVessel");

    vessel.add<navalsim::world::Vessel>();

    vessel.set<navalsim::geography::GeographicPosition>(
        navalsim::geography::GeographicPosition(
            navalsim::geography::Latitude(0.0),
            navalsim::geography::Longitude(0.0),
            navalsim::geography::Elevation(0.0)
        )
    );

    vessel.set<navalsim::movement::Heading>(
        navalsim::movement::Heading(90.0)
    );
    vessel.set<navalsim::movement::DesiredHeading>(
        navalsim::movement::DesiredHeading(90.0)
    );
    vessel.set<navalsim::movement::TurnRate>(
        navalsim::movement::TurnRate(90.0)
    );
    vessel.set<navalsim::movement::TurnDirection>(
        navalsim::movement::TurnDirection::None
    );
    vessel.set<navalsim::movement::Speed>(
        navalsim::movement::Speed(10.0)
    );
    vessel.set<navalsim::movement::MovementStatus>(
        navalsim::movement::MovementStatus::Underway
    );

    return vessel;
}

} // namespace

TEST_CASE("Simulation processes heading commands on the next fixed step")
{
    Simulation simulation;
    flecs::entity vessel;

    REQUIRE(simulation.initialize(
        [&vessel](World& world)
        {
            vessel = createCommandTestVessel(world);
        }
    ));

    simulation.submitMovementCommand(
        navalsim::movement::SetDesiredHeadingCommand{
            vessel.id(),
            navalsim::geography::Bearing(180.0)
        }
    );

    // The command must wait until a complete fixed step is available.
    CHECK(simulation.advance(0.5) == 0);
    CHECK(simulation.takeMovementCommandResults().empty());

    // Completing the fixed step processes the command before the systems run.
    CHECK(simulation.advance(0.5) == 1);

    CHECK(
        vessel.get<navalsim::movement::DesiredHeading>().degrees()
        == doctest::Approx(180.0)
    );
    CHECK(
        vessel.get<navalsim::movement::Heading>().degrees()
        == doctest::Approx(180.0)
    );

    const auto results = simulation.takeMovementCommandResults();

    REQUIRE(results.size() == 1);
	CHECK(results[0].commandId == 1);
    CHECK(results[0].status == navalsim::movement::MovementCommandStatus::Applied);

    // Results are consumed when retrieved.
    CHECK(simulation.takeMovementCommandResults().empty());
}

TEST_CASE("Simulation processes speed commands before vessel movement")
{
    Simulation simulation;
    flecs::entity vessel;

    REQUIRE(simulation.initialize(
        [&vessel](World& world)
        {
            vessel = createCommandTestVessel(world);
        }
    ));

    const auto initialPosition =
        vessel.get<navalsim::geography::GeographicPosition>();

    simulation.submitMovementCommand(
        navalsim::movement::SetSpeedCommand{
            vessel.id(),
            navalsim::movement::Speed(20.0)
        }
    );

    REQUIRE(simulation.advance(1.0) == 1);

    CHECK(
        vessel.get<navalsim::movement::Speed>().metersPerSecond()
        == doctest::Approx(20.0)
    );
    CHECK(
        vessel.get<navalsim::movement::MovementStatus>()
        == navalsim::movement::MovementStatus::Underway
    );

    const auto finalPosition =
        vessel.get<navalsim::geography::GeographicPosition>();

    CHECK(
        navalsim::geography::horizontalDistance(
            initialPosition,
            finalPosition
        ).meters() == doctest::Approx(20.0).epsilon(0.001)
    );

    const auto results = simulation.takeMovementCommandResults();

    REQUIRE(results.size() == 1);
    CHECK(results[0].status == navalsim::movement::MovementCommandStatus::Applied);
}

TEST_CASE("Simulation processes stop commands before vessel movement")
{
    Simulation simulation;
    flecs::entity vessel;

    REQUIRE(simulation.initialize(
        [&vessel](World& world)
        {
            vessel = createCommandTestVessel(world);
        }
    ));

    const auto initialPosition =
        vessel.get<navalsim::geography::GeographicPosition>();

    simulation.submitMovementCommand(
        navalsim::movement::StopMovementCommand{vessel.id()}
    );

    REQUIRE(simulation.advance(1.0) == 1);

    CHECK(
        vessel.get<navalsim::movement::Speed>().metersPerSecond()
        == doctest::Approx(0.0)
    );
    CHECK(
        vessel.get<navalsim::movement::MovementStatus>()
        == navalsim::movement::MovementStatus::Stopped
    );

    const auto finalPosition =
        vessel.get<navalsim::geography::GeographicPosition>();

    CHECK(
        navalsim::geography::horizontalDistance(
            initialPosition,
            finalPosition
        ).meters() == doctest::Approx(0.0)
    );

    const auto results = simulation.takeMovementCommandResults();

    REQUIRE(results.size() == 1);
    CHECK(results[0].status == navalsim::movement::MovementCommandStatus::Applied);
}


TEST_CASE("Simulation processes queued commands once and in submission order")
{
    Simulation simulation;
    flecs::entity vessel;

    REQUIRE(simulation.initialize(
        [&vessel](World& world)
        {
            vessel = createCommandTestVessel(world);
        }
    ));

    using namespace navalsim::movement;

    const auto setSpeedId = simulation.submitMovementCommand(
        SetSpeedCommand{
            vessel.id(),
            Speed(20.0)
        }
    );

    const auto stopId = simulation.submitMovementCommand(
        StopMovementCommand{vessel.id()}
    );

    // Three fixed steps execute, but both commands should be processed
    // only once, at the beginning of the first step.
    REQUIRE(simulation.advance(3.0) == 3);

    // The second command must take effect after the first command.
    CHECK(vessel.get<Speed>().metersPerSecond() == doctest::Approx(0.0));
    CHECK(vessel.get<MovementStatus>() == MovementStatus::Stopped);

    // Both commands must produce exactly one result in submission order.
    const auto results = simulation.takeMovementCommandResults();

    REQUIRE(results.size() == 2);

    CHECK(results[0].commandId == setSpeedId);
    CHECK(results[0].status == MovementCommandStatus::Applied);

    CHECK(results[1].commandId == stopId);
    CHECK(results[1].status == MovementCommandStatus::Applied);

    // No duplicate results should appear on subsequent fixed steps.
    CHECK(simulation.takeMovementCommandResults().empty());
}


TEST_CASE("Simulation discards pending movement commands on shutdown")
{
    Simulation simulation;
    flecs::entity vessel;

    REQUIRE(simulation.initialize(
        [&vessel](World& world)
        {
            vessel = createCommandTestVessel(world);
        }
    ));

    using namespace navalsim::movement;

    const auto commandId = simulation.submitMovementCommand(
        SetSpeedCommand{
            vessel.id(),
            Speed(30.0)
        }
    );

    CHECK(commandId != 0);

    // Shut down before the command can be processed.
    simulation.shutdown();

    // Reinitialize with a fresh world and vessel.
    REQUIRE(simulation.initialize(
        [&vessel](World& world)
        {
            vessel = createCommandTestVessel(world);
        }
    ));

    REQUIRE(simulation.advance(1.0) == 1);

    // The old command must not affect the new world.
    CHECK(vessel.get<Speed>().metersPerSecond() == doctest::Approx(10.0));
    CHECK(vessel.get<MovementStatus>() == MovementStatus::Underway);

    // No result should be produced for the discarded command.
    CHECK(simulation.takeMovementCommandResults().empty());
}

TEST_CASE("Simulation rejects movement commands before initialization")
{
    Simulation simulation;

    CHECK_THROWS_AS(
        simulation.submitMovementCommand(
            navalsim::movement::SetSpeedCommand{
                1,
                navalsim::movement::Speed(10.0)
            }
        ),
        std::logic_error
    );
}

