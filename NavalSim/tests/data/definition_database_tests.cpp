#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

#include "data/definitions/definition_database.hpp"
#include "data/definitions/vessel_definition.hpp"

using namespace navalsim::data;

namespace {

VesselDefinition makeDestroyerDefinition() {
    return VesselDefinition(
        DefinitionId("destroyer.ddg51"),
        "Arleigh Burke-class Destroyer",
        VesselDimensions(
            navalsim::geography::Distance(155.0),
            navalsim::geography::Distance(20.0),
            navalsim::geography::Distance(9.5)
        ),
        VesselPropulsion(30.0)
    );
}

VesselDefinition makeFrigateDefinition() {
    return VesselDefinition(
        DefinitionId("frigate.f100"),
        "Álvaro de Bazán-class Frigate",
        VesselDimensions(
            navalsim::geography::Distance(146.7),
            navalsim::geography::Distance(18.6),
            navalsim::geography::Distance(4.75)
        ),
        VesselPropulsion(28.0)
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

TEST_CASE("Definition database stores vessel definitions") {
    DefinitionDatabase database;
    database.add(makeDestroyerDefinition());

    CHECK(database.definitionCount() == 1);
}

TEST_CASE("Definition database finds a definition by ID and type") {
    DefinitionDatabase database;
    database.add(makeDestroyerDefinition());

    const VesselDefinition* definition =
        database.find<VesselDefinition>(DefinitionId("destroyer.ddg51"));

    REQUIRE(definition != nullptr);
    CHECK(definition->name() == "Arleigh Burke-class Destroyer");
    CHECK(definition->dimensions().length().meters() == doctest::Approx(155.0));
}

TEST_CASE("Definition database returns null for an unknown ID") {
    DefinitionDatabase database;

    CHECK(database.find<VesselDefinition>(DefinitionId("unknown.vessel")) == nullptr);
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

    const VesselDefinition* destroyer =
        database.find<VesselDefinition>(DefinitionId("destroyer.ddg51"));

    const VesselDefinition* frigate =
        database.find<VesselDefinition>(DefinitionId("frigate.f100"));

    REQUIRE(destroyer != nullptr);
    REQUIRE(frigate != nullptr);
    CHECK(destroyer->name() == "Arleigh Burke-class Destroyer");
    CHECK(frigate->name() == "Álvaro de Bazán-class Frigate");
}

TEST_CASE("Definition database owns stored definitions") {
    DefinitionDatabase database;

    {
        VesselDefinition definition = makeDestroyerDefinition();
        database.add(std::move(definition));
    }

    const VesselDefinition* stored =
        database.find<VesselDefinition>(DefinitionId("destroyer.ddg51"));

    REQUIRE(stored != nullptr);
    CHECK(stored->name() == "Arleigh Burke-class Destroyer");
}

TEST_CASE("Definition database exposes definitions as const") {
    using LookupResult = decltype(
        std::declval<const DefinitionDatabase&>().find<VesselDefinition>(
            std::declval<const DefinitionId&>()
        )
    );

    static_assert(std::is_same_v<LookupResult, const VesselDefinition*>);
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

    const VesselDefinition* vessel =
        database.find<VesselDefinition>(DefinitionId("destroyer.ddg51"));

    const AircraftTestDefinition* aircraft =
        database.find<AircraftTestDefinition>(DefinitionId("aircraft.f18"));

    REQUIRE(vessel != nullptr);
    REQUIRE(aircraft != nullptr);

    CHECK(vessel->name() == "Arleigh Burke-class Destroyer");
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
