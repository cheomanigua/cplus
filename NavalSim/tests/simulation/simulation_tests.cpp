#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <cmath>
#include <exception>
#include <limits>
#include <stdexcept>
#include <vector>

#include <doctest/doctest.h>
#include <flecs.h>

#include "core/geography/geographic_types.hpp"
#include "core/geography/geographic_calculations.hpp"
#include "data/definitions/definition_database.hpp"
#include "data/definitions/vessel_definition.hpp"
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

    const navalsim::geography::GeographicPosition initialPosition(
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

			vessel.set<navalsim::movement::MaximumSpeed>(
			    navalsim::movement::MaximumSpeed(25.0)
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

TEST_CASE("Simulation creates a vessel from its loaded definition")
{
    using namespace navalsim::data;
    using namespace navalsim::geography;
    using namespace navalsim::movement;

    DefinitionDatabase definitions;

    definitions.add(
        VesselDefinition(
            DefinitionId("destroyer.ddg51"),
            "Arleigh Burke-class Destroyer",
            VesselDimensions(
                Distance(155.0),
                Distance(20.0),
                Distance(9.5)
            ),
            VesselPropulsion(30.0)
        )
    );

    Simulation simulation;
    World* runtimeWorld = nullptr;

    REQUIRE(simulation.initialize(
        definitions,
        [&runtimeWorld](World& world)
        {
            runtimeWorld = &world;
        }
    ));

    const GeographicPosition initialPosition(
        Latitude(35.0),
        Longitude(-10.0),
        Elevation(15.0)
    );

    const flecs::entity_t vesselId = simulation.createVessel(
        DefinitionId("destroyer.ddg51"),
        initialPosition,
        45.0
    );

    REQUIRE(runtimeWorld != nullptr);

    const flecs::entity vessel = runtimeWorld->ecs().entity(vesselId);

    CHECK(vessel.is_alive());
    CHECK(vessel.has<navalsim::world::Vessel>());

    REQUIRE(vessel.has<GeographicPosition>());
    const auto& position = vessel.get<GeographicPosition>();
    CHECK(position.latitude().degrees() == doctest::Approx(35.0));
    CHECK(position.longitude().degrees() == doctest::Approx(-10.0));
    CHECK(position.elevation().meters() == doctest::Approx(15.0));

    REQUIRE(vessel.has<Heading>());
    CHECK(vessel.get<Heading>().degrees() == doctest::Approx(45.0));

    REQUIRE(vessel.has<DesiredHeading>());
    CHECK(vessel.get<DesiredHeading>().degrees() == doctest::Approx(45.0));

    REQUIRE(vessel.has<Speed>());
    CHECK(vessel.get<Speed>().metersPerSecond() == doctest::Approx(0.0));

    REQUIRE(vessel.has<MaximumSpeed>());
    CHECK(vessel.get<MaximumSpeed>().metersPerSecond() == doctest::Approx(30.0));

    REQUIRE(vessel.has<TurnRate>());
    CHECK(vessel.get<TurnRate>().degreesPerSecond() == doctest::Approx(0.0));

    CHECK(vessel.has<TurnDirection>());
    CHECK(vessel.get<TurnDirection>() == TurnDirection::None);

    CHECK(vessel.has<MovementStatus>());
    CHECK(vessel.get<MovementStatus>() == MovementStatus::Stopped);
}

TEST_CASE("Simulation rejects vessel creation with an unknown definition")
{
    using namespace navalsim::data;
    using namespace navalsim::geography;

    DefinitionDatabase definitions;
    Simulation simulation;

    REQUIRE(simulation.initialize(definitions));

    CHECK_THROWS_AS(
        simulation.createVessel(
            DefinitionId("unknown.vessel"),
            GeographicPosition(
                Latitude(0.0),
                Longitude(0.0),
                Elevation(0.0)
            )
        ),
        std::invalid_argument
    );
}

TEST_CASE("Simulation rejects vessel creation without a definition database")
{
    using namespace navalsim::data;
    using namespace navalsim::geography;

    Simulation simulation;

    REQUIRE(simulation.initialize());

    CHECK_THROWS_AS(
        simulation.createVessel(
            DefinitionId("destroyer.ddg51"),
            GeographicPosition(
                Latitude(0.0),
                Longitude(0.0),
                Elevation(0.0)
            )
        ),
        std::logic_error
    );
}

TEST_CASE("Simulation time acceleration affects vessel movement")
{
    Simulation simulation;

    flecs::entity vessel;

    const navalsim::geography::GeographicPosition initialPosition(
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

			vessel.set<navalsim::movement::MaximumSpeed>(
			    navalsim::movement::MaximumSpeed(25.0)
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
				vessel.set<MaximumSpeed>(MaximumSpeed(25.0));
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
    vessel.set<navalsim::movement::MaximumSpeed>(
        navalsim::movement::MaximumSpeed(25.0)
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

TEST_CASE("Simulation accepts generic movement command submission")
{
    Simulation simulation;
    flecs::entity vessel;

    REQUIRE(simulation.initialize(
        [&vessel](World& world)
        {
            vessel = createCommandTestVessel(world);
        }
    ));

    const auto commandId = simulation.submitCommand(
        navalsim::movement::SetSpeedCommand{
            vessel.id(),
            navalsim::movement::Speed(25.0)
        }
    );

    CHECK(commandId != 0);
    REQUIRE(simulation.advance(1.0) == 1);

    CHECK(vessel.get<navalsim::movement::Speed>().metersPerSecond()
        == doctest::Approx(25.0));

    const auto results = simulation.takeCommandResults();

    REQUIRE(results.size() == 1);
    CHECK(results[0].commandId == commandId);
    CHECK(results[0].status == navalsim::simulation::SimulationCommandStatus::Applied);
}

TEST_CASE("Simulation generic command results are consumed when retrieved")
{
    Simulation simulation;
    flecs::entity vessel;

    REQUIRE(simulation.initialize(
        [&vessel](World& world)
        {
            vessel = createCommandTestVessel(world);
        }
    ));

    simulation.submitCommand(
        navalsim::movement::StopMovementCommand{vessel.id()}
    );

    REQUIRE(simulation.advance(1.0) == 1);

    const auto results = simulation.takeCommandResults();

    REQUIRE(results.size() == 1);
    CHECK(results[0].status == navalsim::simulation::SimulationCommandStatus::Applied);
    CHECK(simulation.takeCommandResults().empty());
}

TEST_CASE("Movement compatibility methods share the generic command queue and results")
{
    Simulation simulation;
    flecs::entity vessel;

    REQUIRE(simulation.initialize(
        [&vessel](World& world)
        {
            vessel = createCommandTestVessel(world);
        }
    ));

    const auto movementCommandId = simulation.submitMovementCommand(
        navalsim::movement::SetSpeedCommand{
            vessel.id(),
            navalsim::movement::Speed(15.0)
        }
    );

    REQUIRE(simulation.advance(1.0) == 1);

    const auto genericResults = simulation.takeCommandResults();

    REQUIRE(genericResults.size() == 1);
    CHECK(genericResults[0].commandId == movementCommandId);
    CHECK(genericResults[0].status == navalsim::simulation::SimulationCommandStatus::Applied);

    const auto genericCommandId = simulation.submitCommand(
        navalsim::movement::StopMovementCommand{vessel.id()}
    );

    REQUIRE(simulation.advance(1.0) == 1);

    const auto movementResults = simulation.takeMovementCommandResults();

    REQUIRE(movementResults.size() == 1);
    CHECK(movementResults[0].commandId == genericCommandId);
    CHECK(movementResults[0].status == navalsim::movement::MovementCommandStatus::Applied);
}

TEST_CASE("Simulation rejects generic commands before initialization")
{
    Simulation simulation;

    CHECK_THROWS_AS(
        simulation.submitCommand(
            navalsim::movement::SetSpeedCommand{
                1,
                navalsim::movement::Speed(10.0)
            }
        ),
        std::logic_error
    );
}

TEST_CASE("Simulation dispatches commands from independent domains")
{
    struct SensorCalibrationCommand
    {
        int calibrationValue;
    };

    enum class SensorCalibrationStatus
    {
        Calibrated
    };

    Simulation simulation;
    CHECK(simulation.initialize());

    int appliedCalibration = 0;

    simulation.registerCommandHandler<
        SensorCalibrationCommand,
        SensorCalibrationStatus
    >(
        [&appliedCalibration](
            Simulation&,
            const SensorCalibrationCommand& command
        ) {
            appliedCalibration = command.calibrationValue;
            return SensorCalibrationStatus::Calibrated;
        }
    );

    const auto commandId = simulation.submitCommand<
        SensorCalibrationCommand,
        SensorCalibrationStatus
    >({42});

    CHECK(commandId != 0);
    CHECK(appliedCalibration == 0);

    CHECK(simulation.advance(1.0) == 1);
    CHECK(appliedCalibration == 42);

    const auto results =
        simulation.takeCommandResults<SensorCalibrationStatus>();

    REQUIRE(results.size() == 1);
    CHECK(results.front().commandId == commandId);
    CHECK(results.front().status == SensorCalibrationStatus::Calibrated);
}

TEST_CASE("Simulation continues processing commands after a handler throws")
{
    struct FailingCommand
    {
        int value;
    };

    struct SuccessfulCommand
    {
        int value;
    };

    enum class FailingStatus
    {
        Completed
    };

    enum class SuccessfulStatus
    {
        Applied
    };

    Simulation simulation;
    REQUIRE(simulation.initialize());

    std::vector<int> executionOrder;
    int successfulValue = 0;

    simulation.registerCommandHandler<FailingCommand, FailingStatus>(
        [&executionOrder](Simulation&, const FailingCommand& command) {
            executionOrder.push_back(command.value);
            throw std::runtime_error("Intentional test failure.");
            return FailingStatus::Completed;
        }
    );

    simulation.registerCommandHandler<SuccessfulCommand, SuccessfulStatus>(
        [&executionOrder, &successfulValue](Simulation&, const SuccessfulCommand& command) {
            executionOrder.push_back(command.value);
            successfulValue = command.value;
            return SuccessfulStatus::Applied;
        }
    );

    const auto failedId = simulation.submitCommand<FailingCommand, FailingStatus>({1});
    const auto successfulId = simulation.submitCommand<SuccessfulCommand, SuccessfulStatus>({2});

    REQUIRE(simulation.advance(1.0) == 1);

    CHECK(executionOrder == std::vector<int>{1, 2});
    CHECK(successfulValue == 2);

    const auto successes = simulation.takeCommandResults<SuccessfulStatus>();
    REQUIRE(successes.size() == 1);
    CHECK(successes.front().commandId == successfulId);
    CHECK(successes.front().status == SuccessfulStatus::Applied);

    const auto failures = simulation.takeCommandFailures();
    REQUIRE(failures.size() == 1);
    CHECK(failures.front().commandId == failedId);
    CHECK(failures.front().message == "Intentional test failure.");
    CHECK_THROWS_AS(
        std::rethrow_exception(failures.front().exception),
        std::runtime_error
    );

    // The failed command must not be retried on a later simulation step.
    REQUIRE(simulation.advance(1.0) == 1);
    CHECK(executionOrder == std::vector<int>{1, 2});
    CHECK(simulation.takeCommandFailures().empty());
}

TEST_CASE("Simulation captures non-standard command handler exceptions")
{
    struct UnusualFailureCommand
    {
        int value;
    };

    enum class UnusualFailureStatus
    {
        Completed
    };

    Simulation simulation;
    REQUIRE(simulation.initialize());

    simulation.registerCommandHandler<UnusualFailureCommand, UnusualFailureStatus>(
        [](Simulation&, const UnusualFailureCommand&) -> UnusualFailureStatus {
            throw 42;
        }
    );

    const auto commandId =
        simulation.submitCommand<UnusualFailureCommand, UnusualFailureStatus>({42});

    REQUIRE(simulation.advance(1.0) == 1);

    const auto failures = simulation.takeCommandFailures();

    REQUIRE(failures.size() == 1);
    CHECK(failures.front().commandId == commandId);
    CHECK(failures.front().message == "Command handler threw a non-standard exception.");
    CHECK_THROWS_AS(
        std::rethrow_exception(failures.front().exception),
        int
    );
}

TEST_CASE("Simulation preserves FIFO ordering across domains and isolates typed results")
{
    struct NavigationCommand { int value; };
    struct SensorCommand { int value; };
    struct WeaponCommand { int value; };

    enum class NavigationStatus { Applied };
    enum class SensorStatus { Applied };
    enum class WeaponStatus { Applied };

    Simulation simulation;
    REQUIRE(simulation.initialize());

    std::vector<int> executionOrder;

    simulation.registerCommandHandler<NavigationCommand, NavigationStatus>(
        [&executionOrder](Simulation&, const NavigationCommand& command) {
            executionOrder.push_back(command.value);
            return NavigationStatus::Applied;
        }
    );

    simulation.registerCommandHandler<SensorCommand, SensorStatus>(
        [&executionOrder](Simulation&, const SensorCommand& command) -> SensorStatus {
            executionOrder.push_back(command.value);
            throw std::runtime_error("Sensor command failed.");
        }
    );

    simulation.registerCommandHandler<WeaponCommand, WeaponStatus>(
        [&executionOrder](Simulation&, const WeaponCommand& command) {
            executionOrder.push_back(command.value);
            return WeaponStatus::Applied;
        }
    );

    const auto navigationId = simulation.submitCommand<NavigationCommand, NavigationStatus>({1});
    const auto sensorId = simulation.submitCommand<SensorCommand, SensorStatus>({2});
    const auto weaponId = simulation.submitCommand<WeaponCommand, WeaponStatus>({3});

    REQUIRE(simulation.advance(1.0) == 1);

    // All domains execute in global submission order, including the failed command.
    CHECK(executionOrder == std::vector<int>{1, 2, 3});

    // Retrieving weapon results must not consume navigation results.
    const auto weaponResults = simulation.takeCommandResults<WeaponStatus>();
    REQUIRE(weaponResults.size() == 1);
    CHECK(weaponResults.front().commandId == weaponId);
    CHECK(weaponResults.front().status == WeaponStatus::Applied);
    CHECK(simulation.takeCommandResults<WeaponStatus>().empty());

    const auto navigationResults = simulation.takeCommandResults<NavigationStatus>();
    REQUIRE(navigationResults.size() == 1);
    CHECK(navigationResults.front().commandId == navigationId);
    CHECK(navigationResults.front().status == NavigationStatus::Applied);

    const auto failures = simulation.takeCommandFailures();
    REQUIRE(failures.size() == 1);
    CHECK(failures.front().commandId == sensorId);
    CHECK(failures.front().message == "Sensor command failed.");

    CHECK(simulation.takeCommandResults<NavigationStatus>().empty());
    CHECK(simulation.takeCommandResults<WeaponStatus>().empty());
    CHECK(simulation.takeCommandFailures().empty());
}

TEST_CASE("Commands submitted during command processing wait for the next fixed step")
{
    struct InitialCommand { int value; };
    struct DeferredCommand { int value; };

    enum class InitialStatus { Applied };
    enum class DeferredStatus { Applied };

    Simulation simulation;
    REQUIRE(simulation.initialize());

    std::vector<int> executionOrder;

    simulation.registerCommandHandler<InitialCommand, InitialStatus>(
        [&simulation, &executionOrder](Simulation&, const InitialCommand& command) {
            executionOrder.push_back(command.value);

            // This command is submitted while the current batch is executing.
            simulation.submitCommand<DeferredCommand, DeferredStatus>({2});

            return InitialStatus::Applied;
        }
    );

    simulation.registerCommandHandler<DeferredCommand, DeferredStatus>(
        [&executionOrder](Simulation&, const DeferredCommand& command) {
            executionOrder.push_back(command.value);
            return DeferredStatus::Applied;
        }
    );

    const auto initialId =
        simulation.submitCommand<InitialCommand, InitialStatus>({1});

    REQUIRE(simulation.advance(1.0) == 1);

    // The command submitted by the handler must not run in the current batch.
    CHECK(executionOrder == std::vector<int>{1});

    const auto initialResults = simulation.takeCommandResults<InitialStatus>();
    REQUIRE(initialResults.size() == 1);
    CHECK(initialResults.front().commandId == initialId);

    CHECK(simulation.takeCommandResults<DeferredStatus>().empty());

    // The deferred command runs at the beginning of the next fixed step.
    REQUIRE(simulation.advance(1.0) == 1);
    CHECK(executionOrder == std::vector<int>{1, 2});

    const auto deferredResults = simulation.takeCommandResults<DeferredStatus>();
    REQUIRE(deferredResults.size() == 1);
    CHECK(deferredResults.front().status == DeferredStatus::Applied);
}

TEST_CASE("Simulation preserves command IDs and handlers across shutdown")
{
    struct LifecycleCommand { int value; };
    enum class LifecycleStatus { Applied };

    Simulation simulation;
    REQUIRE(simulation.initialize());

    int executionCount = 0;

    simulation.registerCommandHandler<LifecycleCommand, LifecycleStatus>(
        [&executionCount](Simulation&, const LifecycleCommand&) {
            ++executionCount;
            return LifecycleStatus::Applied;
        }
    );

    // This command is discarded by shutdown, but its ID must not be reused.
    const auto firstId =
        simulation.submitCommand<LifecycleCommand, LifecycleStatus>({1});

    simulation.shutdown();

    CHECK(simulation.takeCommandResults<LifecycleStatus>().empty());
    CHECK(simulation.takeCommandFailures().empty());

    REQUIRE(simulation.initialize());

    // The handler remains registered, and the queue's ID counter is preserved.
    const auto secondId =
        simulation.submitCommand<LifecycleCommand, LifecycleStatus>({2});

    CHECK(secondId > firstId);

    REQUIRE(simulation.advance(1.0) == 1);
    CHECK(executionCount == 1);

    const auto results = simulation.takeCommandResults<LifecycleStatus>();
    REQUIRE(results.size() == 1);
    CHECK(results.front().commandId == secondId);
    CHECK(results.front().status == LifecycleStatus::Applied);

    // Results that have not been collected are discarded by shutdown.
    simulation.submitCommand<LifecycleCommand, LifecycleStatus>({3});
    REQUIRE(simulation.advance(1.0) == 1);
    CHECK(executionCount == 2);

    simulation.shutdown();

    CHECK(simulation.takeCommandResults<LifecycleStatus>().empty());
    CHECK(simulation.takeCommandFailures().empty());
}

TEST_CASE("Simulation allows registering a new handler during command execution")
{
    struct RegistrationCommand {};
    struct NewlyRegisteredCommand { int value; };

    enum class RegistrationStatus { Applied };
    enum class NewlyRegisteredStatus { Applied };

    Simulation simulation;
    REQUIRE(simulation.initialize());

    std::vector<int> executionOrder;

    simulation.registerCommandHandler<RegistrationCommand, RegistrationStatus>(
        [&simulation, &executionOrder](Simulation&, const RegistrationCommand&) {
            executionOrder.push_back(1);

            simulation.registerCommandHandler<
                NewlyRegisteredCommand,
                NewlyRegisteredStatus
            >(
                [&executionOrder](
                    Simulation&,
                    const NewlyRegisteredCommand& command
                ) {
                    executionOrder.push_back(command.value);
                    return NewlyRegisteredStatus::Applied;
                }
            );

            // Registration happens before submission, so submission is accepted.
            simulation.submitCommand<NewlyRegisteredCommand, NewlyRegisteredStatus>(
                {2}
            );

            return RegistrationStatus::Applied;
        }
    );

    simulation.submitCommand<RegistrationCommand, RegistrationStatus>({});

    REQUIRE(simulation.advance(1.0) == 1);

    // The newly submitted command belongs to the next processing batch.
    CHECK(executionOrder == std::vector<int>{1});

    REQUIRE(simulation.advance(1.0) == 1);
    CHECK(executionOrder == std::vector<int>{1, 2});

    const auto results =
        simulation.takeCommandResults<NewlyRegisteredStatus>();

    REQUIRE(results.size() == 1);
    CHECK(results.front().status == NewlyRegisteredStatus::Applied);
}
