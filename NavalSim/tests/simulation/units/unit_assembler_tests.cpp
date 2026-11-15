#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <cmath>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

#include <doctest/doctest.h>

#include "core/geography/geographic_types.hpp"
#include "data/definitions/component_definition.hpp"
#include "data/definitions/definition_database.hpp"
#include "data/definitions/definition_id.hpp"
#include "data/definitions/unit_definition.hpp"
#include "domains/fuel/fuel_components.hpp"
#include "domains/movement/movement_components.hpp"
#include "simulation/units/component_installation_components.hpp"
#include "simulation/units/unit_assembler.hpp"
#include "world/world.hpp"

namespace
{

using navalsim::data::ComponentCategory;
using navalsim::data::ComponentDefinition;
using navalsim::data::DefinitionDatabase;
using navalsim::data::DefinitionId;
using navalsim::data::FuelProperties;
using navalsim::data::RadarProperties;
using navalsim::data::UnitComponentReference;
using navalsim::data::UnitDefinition;
using navalsim::data::UnitEnvironment;
using navalsim::geography::Elevation;
using navalsim::geography::GeographicPosition;
using navalsim::geography::Latitude;
using navalsim::geography::Longitude;
using navalsim::movement::DesiredHeading;
using navalsim::movement::Heading;
using navalsim::movement::MaximumSpeed;
using navalsim::movement::MovementStatus;
using navalsim::movement::Speed;
using navalsim::movement::TurnDirection;
using navalsim::movement::TurnRate;
using navalsim::simulation::ComponentCondition;
using navalsim::simulation::ComponentInstallation;
using navalsim::simulation::RadarConfiguration;
using navalsim::simulation::RadarOperationalState;
using navalsim::simulation::UnitAssembler;

constexpr double metersPerSecondPerKnot = 1852.0 / 3600.0;

GeographicPosition makePosition()
{
    return GeographicPosition(
        Latitude(35.0),
        Longitude(140.0),
        Elevation(25.0)
    );
}

UnitDefinition makeUnitDefinition(
    double maxSpeedKnots = 30.0,
    std::optional<FuelProperties> fuel = std::nullopt,
    std::vector<UnitComponentReference> components = {}
)
{
    return UnitDefinition(
        DefinitionId("test_unit"),
        "Test Unit",
        "ship",
        UnitEnvironment::Water,
        maxSpeedKnots,
        std::move(fuel),
        std::move(components)
    );
}

DefinitionDatabase makeDefinitions(double maxSpeedKnots = 30.0)
{
    DefinitionDatabase definitions;
    definitions.add<UnitDefinition>(makeUnitDefinition(maxSpeedKnots));
    return definitions;
}

std::vector<flecs::entity_t> getChildren(
    World& world,
    flecs::entity_t parentId
)
{
    std::vector<flecs::entity_t> children;

    world.ecs().entity(parentId).children(
        [&children](flecs::entity child)
        {
            children.push_back(child.id());
        }
    );

    return children;
}

} // namespace

TEST_CASE("UnitAssembler creates a runtime unit")
{
    const auto definitions = makeDefinitions();
    World world;
    UnitAssembler assembler;

    const auto entityId = assembler.assemble(
        definitions,
        world,
        DefinitionId("test_unit"),
        makePosition()
    );

    const auto entity = world.ecs().entity(entityId);

    CHECK(entity.is_alive());
    CHECK(entity.has<GeographicPosition>());
    CHECK(entity.has<Heading>());
    CHECK(entity.has<DesiredHeading>());
    CHECK(entity.has<Speed>());
    CHECK(entity.has<MaximumSpeed>());
    CHECK(entity.has<TurnRate>());
    CHECK(entity.has<TurnDirection>());
    CHECK(entity.has<MovementStatus>());
}

TEST_CASE("UnitAssembler preserves the initial geographic position")
{
    const auto definitions = makeDefinitions();
    World world;
    UnitAssembler assembler;

    const auto position = makePosition();

    const auto entityId = assembler.assemble(
        definitions,
        world,
        DefinitionId("test_unit"),
        position
    );

    const auto entity = world.ecs().entity(entityId);
    const auto actualPosition = entity.get<GeographicPosition>();

    CHECK(actualPosition.latitude().degrees() == doctest::Approx(35.0));
    CHECK(actualPosition.longitude().degrees() == doctest::Approx(140.0));
    CHECK(actualPosition.elevation().meters() == doctest::Approx(25.0));
}

TEST_CASE("UnitAssembler initializes heading and desired heading")
{
    const auto definitions = makeDefinitions();
    World world;
    UnitAssembler assembler;

    const auto entityId = assembler.assemble(
        definitions,
        world,
        DefinitionId("test_unit"),
        makePosition(),
        725.0
    );

    const auto entity = world.ecs().entity(entityId);

    const auto heading = entity.get<Heading>();
    const auto desiredHeading = entity.get<DesiredHeading>();

    CHECK(heading.degrees() == doctest::Approx(5.0));
    CHECK(desiredHeading.degrees() == doctest::Approx(5.0));
}

TEST_CASE("UnitAssembler initializes movement state")
{
    const auto definitions = makeDefinitions();
    World world;
    UnitAssembler assembler;

    const auto entityId = assembler.assemble(
        definitions,
        world,
        DefinitionId("test_unit"),
        makePosition()
    );

    const auto entity = world.ecs().entity(entityId);

    const auto speed = entity.get<Speed>();
    const auto turnRate = entity.get<TurnRate>();
    const auto turnDirection = entity.get<TurnDirection>();
    const auto movementStatus = entity.get<MovementStatus>();

    CHECK(speed.metersPerSecond() == doctest::Approx(0.0));
    CHECK(turnRate.degreesPerSecond() == doctest::Approx(0.0));
    CHECK(turnDirection == TurnDirection::None);
    CHECK(movementStatus == MovementStatus::Stopped);
}

TEST_CASE("UnitAssembler converts maximum speed from knots to meters per second")
{
    const auto definitions = makeDefinitions(30.0);
    World world;
    UnitAssembler assembler;

    const auto entityId = assembler.assemble(
        definitions,
        world,
        DefinitionId("test_unit"),
        makePosition()
    );

    const auto entity = world.ecs().entity(entityId);
    const auto maximumSpeed = entity.get<MaximumSpeed>();

    CHECK(
        maximumSpeed.metersPerSecond()
        == doctest::Approx(30.0 * metersPerSecondPerKnot)
    );
}

TEST_CASE("UnitAssembler supports units with zero maximum speed")
{
    const auto definitions = makeDefinitions(0.0);
    World world;
    UnitAssembler assembler;

    const auto entityId = assembler.assemble(
        definitions,
        world,
        DefinitionId("test_unit"),
        makePosition()
    );

    const auto entity = world.ecs().entity(entityId);
    const auto maximumSpeed = entity.get<MaximumSpeed>();

    CHECK(maximumSpeed.metersPerSecond() == doctest::Approx(0.0));
}

TEST_CASE("UnitAssembler does not add fuel state to units without fuel")
{
    const auto definitions = makeDefinitions();
    World world;
    UnitAssembler assembler;

    const auto entityId = assembler.assemble(
        definitions,
        world,
        DefinitionId("test_unit"),
        makePosition()
    );

    const auto entity = world.ecs().entity(entityId);

    CHECK_FALSE(entity.has<navalsim::fuel::FuelState>());
}

TEST_CASE("UnitAssembler initializes fuel state at full capacity")
{
    DefinitionDatabase definitions;

    definitions.add<UnitDefinition>(
        makeUnitDefinition(
            30.0,
            FuelProperties{500000.0, 15.0, 2000.0}
        )
    );

    World world;
    UnitAssembler assembler;

    const auto entityId = assembler.assemble(
        definitions,
        world,
        DefinitionId("test_unit"),
        makePosition()
    );

    const auto entity = world.ecs().entity(entityId);
    const auto fuel = entity.get<navalsim::fuel::FuelState>();

    CHECK(fuel.capacity == doctest::Approx(500000.0));
    CHECK(fuel.quantity == doctest::Approx(500000.0));
    CHECK(fuel.referenceSpeedKnots == doctest::Approx(15.0));
    CHECK(fuel.consumptionPerHour == doctest::Approx(2000.0));
}

TEST_CASE("UnitAssembler creates one child entity per component installation")
{
    DefinitionDatabase definitions;

    definitions.add<ComponentDefinition>(
        ComponentDefinition(
            DefinitionId("marine_engine"),
            "Marine Engine",
            ComponentCategory::DamageOnly,
            "engine"
        )
    );

    definitions.add<UnitDefinition>(
        makeUnitDefinition(
            30.0,
            std::nullopt,
            {
                UnitComponentReference(DefinitionId("marine_engine"), 2)
            }
        )
    );

    World world;
    UnitAssembler assembler;

    const auto unitId = assembler.assemble(
        definitions,
        world,
        DefinitionId("test_unit"),
        makePosition()
    );

    const auto children = getChildren(world, unitId);

    REQUIRE(children.size() == 2);

    for (const auto childId : children)
    {
        const auto child = world.ecs().entity(childId);

        REQUIRE(child.is_alive());
        CHECK(child.has<ComponentInstallation>());
        CHECK(child.has<ComponentCondition>());
        CHECK_FALSE(child.has<RadarConfiguration>());
        CHECK_FALSE(child.has<RadarOperationalState>());

        const auto installation = child.get<ComponentInstallation>();
        const auto condition = child.get<ComponentCondition>();

        CHECK(installation.definitionId == "marine_engine");
        CHECK(condition == ComponentCondition::Operational);
    }
}

TEST_CASE("UnitAssembler creates independent component installation states")
{
    DefinitionDatabase definitions;

    definitions.add<ComponentDefinition>(
        ComponentDefinition(
            DefinitionId("marine_engine"),
            "Marine Engine",
            ComponentCategory::DamageOnly,
            "engine"
        )
    );

    definitions.add<UnitDefinition>(
        makeUnitDefinition(
            30.0,
            std::nullopt,
            {
                UnitComponentReference(DefinitionId("marine_engine"), 2)
            }
        )
    );

    World world;
    UnitAssembler assembler;

    const auto unitId = assembler.assemble(
        definitions,
        world,
        DefinitionId("test_unit"),
        makePosition()
    );

    const auto children = getChildren(world, unitId);

    REQUIRE(children.size() == 2);

    auto first = world.ecs().entity(children[0]);
    auto second = world.ecs().entity(children[1]);

    first.set<ComponentCondition>(ComponentCondition::Damaged);

    CHECK(
        first.get<ComponentCondition>()
        == ComponentCondition::Damaged
    );

    CHECK(
        second.get<ComponentCondition>()
        == ComponentCondition::Operational
    );
}

TEST_CASE("UnitAssembler copies radar configuration into its installation")
{
    DefinitionDatabase definitions;

    definitions.add<ComponentDefinition>(
        ComponentDefinition(
            DefinitionId("search_radar"),
            "Search Radar",
            ComponentCategory::Simulation,
            "radar",
            navalsim::data::ComponentProperties{
                RadarProperties(120.0)
            }
        )
    );

    definitions.add<UnitDefinition>(
        makeUnitDefinition(
            30.0,
            std::nullopt,
            {
                UnitComponentReference(DefinitionId("search_radar"), 1)
            }
        )
    );

    World world;
    UnitAssembler assembler;

    const auto unitId = assembler.assemble(
        definitions,
        world,
        DefinitionId("test_unit"),
        makePosition()
    );

    const auto children = getChildren(world, unitId);

    REQUIRE(children.size() == 1);

    const auto radar = world.ecs().entity(children.front());

    REQUIRE(radar.has<RadarConfiguration>());
    REQUIRE(radar.has<RadarOperationalState>());

    const auto configuration = radar.get<RadarConfiguration>();
    const auto operationalState = radar.get<RadarOperationalState>();

    CHECK(configuration.rangeNm == doctest::Approx(120.0));
    CHECK(operationalState == RadarOperationalState::Enabled);
    CHECK(
        radar.get<ComponentCondition>()
        == ComponentCondition::Operational
    );
}

TEST_CASE("UnitAssembler deletes component installations with their parent unit")
{
    DefinitionDatabase definitions;

    definitions.add<ComponentDefinition>(
        ComponentDefinition(
            DefinitionId("marine_engine"),
            "Marine Engine",
            ComponentCategory::DamageOnly,
            "engine"
        )
    );

    definitions.add<UnitDefinition>(
        makeUnitDefinition(
            30.0,
            std::nullopt,
            {
                UnitComponentReference(DefinitionId("marine_engine"), 1)
            }
        )
    );

    World world;
    UnitAssembler assembler;

    const auto unitId = assembler.assemble(
        definitions,
        world,
        DefinitionId("test_unit"),
        makePosition()
    );

    const auto children = getChildren(world, unitId);

    REQUIRE(children.size() == 1);

    const auto childId = children.front();

    world.ecs().entity(unitId).destruct();

    CHECK_FALSE(world.ecs().is_alive(unitId));
    CHECK_FALSE(world.ecs().is_alive(childId));
}

TEST_CASE("UnitAssembler rejects an unknown unit definition")
{
    const auto definitions = makeDefinitions();
    World world;
    UnitAssembler assembler;

    CHECK_THROWS_AS(
        assembler.assemble(
            definitions,
            world,
            DefinitionId("unknown_unit"),
            makePosition()
        ),
        std::invalid_argument
    );
}

TEST_CASE("UnitAssembler rejects an unknown component definition")
{
    DefinitionDatabase definitions;

    definitions.add<UnitDefinition>(
        makeUnitDefinition(
            30.0,
            std::nullopt,
            {
                UnitComponentReference(DefinitionId("unknown_component"), 1)
            }
        )
    );

    World world;
    UnitAssembler assembler;

    CHECK_THROWS_AS(
        assembler.assemble(
            definitions,
            world,
            DefinitionId("test_unit"),
            makePosition()
        ),
        std::invalid_argument
    );
}
