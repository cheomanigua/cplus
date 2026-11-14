
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

#include "data/definitions/definition_database.hpp"
#include "data/definitions/ammunition_definition.hpp"
#include "data/definitions/component_definition.hpp"
#include "data/definitions/unit_definition.hpp"

using namespace navalsim::data;

namespace {

UnitDefinition makeDestroyerDefinition() {
    return UnitDefinition(
        DefinitionId("destroyer.ddg51"),
        "Arleigh Burke-class Destroyer",
        "destroyer",
        UnitEnvironment::Water,
        30.0,
        std::nullopt,
        {}
    );
}

UnitDefinition makeFrigateDefinition() {
    return UnitDefinition(
        DefinitionId("frigate.f100"),
        "Álvaro de Bazán-class Frigate",
        "frigate",
        UnitEnvironment::Water,
        28.0,
        std::nullopt,
        {}
    );
}

class AircraftTestDefinition {
public:
    AircraftTestDefinition(DefinitionId id, std::string name)
        : id_(std::move(id)), name_(std::move(name)) {}

    const DefinitionId& id() const noexcept {
        return id_;
    }

    const std::string& name() const noexcept {
        return name_;
    }

private:
    DefinitionId id_;
    std::string name_;
};

}

TEST_CASE("Definition database starts empty") {
    const DefinitionDatabase database;
    CHECK(database.definitionCount() == 0);
}

TEST_CASE("Definition database stores unit definitions") {
    DefinitionDatabase database;
    database.add(makeDestroyerDefinition());

    CHECK(database.definitionCount() == 1);
}

TEST_CASE("Definition database finds a definition by ID and type") {
    DefinitionDatabase database;
    database.add(makeDestroyerDefinition());

    const UnitDefinition* definition =
        database.find<UnitDefinition>(DefinitionId("destroyer.ddg51"));

    REQUIRE(definition != nullptr);
    CHECK(definition->name() == "Arleigh Burke-class Destroyer");
    CHECK(definition->maxSpeedKnots() == doctest::Approx(30.0));
}

TEST_CASE("Definition database returns null for an unknown ID") {
    DefinitionDatabase database;

    CHECK(database.find<UnitDefinition>(DefinitionId("unknown.unit")) == nullptr);
}

TEST_CASE("Definition database rejects duplicate IDs") {
    DefinitionDatabase database;
    database.add(makeDestroyerDefinition());

    CHECK_THROWS_AS(database.add(makeDestroyerDefinition()), std::invalid_argument);
    CHECK(database.definitionCount() == 1);
}

TEST_CASE("Definition database stores multiple definitions of the same type") {
    DefinitionDatabase database;
    database.add(makeDestroyerDefinition());
    database.add(makeFrigateDefinition());

    REQUIRE(database.definitionCount() == 2);

    const UnitDefinition* destroyer =
        database.find<UnitDefinition>(DefinitionId("destroyer.ddg51"));

    const UnitDefinition* frigate =
        database.find<UnitDefinition>(DefinitionId("frigate.f100"));

    REQUIRE(destroyer != nullptr);
    REQUIRE(frigate != nullptr);
    CHECK(destroyer->name() == "Arleigh Burke-class Destroyer");
    CHECK(frigate->name() == "Álvaro de Bazán-class Frigate");
}

TEST_CASE("Definition database owns stored definitions") {
    DefinitionDatabase database;

    {
        UnitDefinition definition = makeDestroyerDefinition();
        database.add(std::move(definition));
    }

    const UnitDefinition* stored =
        database.find<UnitDefinition>(DefinitionId("destroyer.ddg51"));

    REQUIRE(stored != nullptr);
    CHECK(stored->name() == "Arleigh Burke-class Destroyer");
}

TEST_CASE("Definition database exposes definitions as const") {
    using LookupResult = decltype(
        std::declval<const DefinitionDatabase&>().find<UnitDefinition>(
            std::declval<const DefinitionId&>()
        )
    );

    static_assert(std::is_same_v<LookupResult, const UnitDefinition*>);
    CHECK(true);
}

TEST_CASE("Definition database supports different definition types") {
    DefinitionDatabase database;

    database.add(makeDestroyerDefinition());
    database.add(AircraftTestDefinition(
        DefinitionId("aircraft.f18"),
        "F/A-18 Hornet"
    ));

    REQUIRE(database.definitionCount() == 2);

    const UnitDefinition* unit =
        database.find<UnitDefinition>(DefinitionId("destroyer.ddg51"));

    const AircraftTestDefinition* aircraft =
        database.find<AircraftTestDefinition>(DefinitionId("aircraft.f18"));

    REQUIRE(unit != nullptr);
    REQUIRE(aircraft != nullptr);

    CHECK(unit->name() == "Arleigh Burke-class Destroyer");
    CHECK(aircraft->name() == "F/A-18 Hornet");
}

TEST_CASE("Definition database rejects lookup using the wrong type") {
    DefinitionDatabase database;
    database.add(makeDestroyerDefinition());

    CHECK(database.find<AircraftTestDefinition>(
        DefinitionId("destroyer.ddg51")
    ) == nullptr);
}

TEST_CASE("Definition IDs are unique across different definition types") {
    DefinitionDatabase database;

    database.add(makeDestroyerDefinition());

    CHECK_THROWS_AS(
        database.add(AircraftTestDefinition(
            DefinitionId("destroyer.ddg51"),
            "Aircraft with duplicate ID"
        )),
        std::invalid_argument
    );

    CHECK(database.definitionCount() == 1);
}

TEST_CASE("Definition database shares one ID namespace across unit component and ammunition definitions") {
    DefinitionDatabase database;

    database.add(UnitDefinition(
        DefinitionId("destroyer"),
        "Destroyer",
        "ship",
        UnitEnvironment::Water,
        30.0,
        FuelProperties{100000.0, 20.0, 1000.0},
        {}
    ));
    database.add(ComponentDefinition(
        DefinitionId("marine_engine"),
        "Marine Engine",
        ComponentCategory::DamageOnly,
        "engine"
    ));
    database.add(AmmunitionDefinition(
        DefinitionId("sidewinder"),
        "Sidewinder",
        18.0,
        850.0,
        true,
        80.0
    ));

    REQUIRE(database.definitionCount() == 3);
    CHECK(database.find<UnitDefinition>(DefinitionId("destroyer")) != nullptr);
    CHECK(database.find<ComponentDefinition>(DefinitionId("marine_engine")) != nullptr);
    CHECK(database.find<AmmunitionDefinition>(DefinitionId("sidewinder")) != nullptr);

    CHECK_THROWS_AS(
        database.add(AmmunitionDefinition(
            DefinitionId("marine_engine"),
            "Conflicting ammunition ID",
            1.0,
            1.0,
            false,
            1.0
        )),
        std::invalid_argument
    );
    CHECK(database.definitionCount() == 3);
}

