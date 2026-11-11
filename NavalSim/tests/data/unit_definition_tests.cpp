
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#include "data/definitions/unit_definition.hpp"

using namespace navalsim::data;

TEST_CASE("Unit physical properties store mass and dimensions") {
    const UnitPhysicalProperties properties(4500000.0, 150.0, 20.0);
    CHECK(properties.massKg() == doctest::Approx(4500000.0));
    CHECK(properties.lengthMeters() == doctest::Approx(150.0));
    CHECK(properties.widthMeters() == doctest::Approx(20.0));
}

TEST_CASE("Unit physical properties reject non-positive and non-finite values") {
    CHECK_THROWS_AS(UnitPhysicalProperties(0.0, 150.0, 20.0), std::invalid_argument);
    CHECK_THROWS_AS(UnitPhysicalProperties(1.0, -1.0, 20.0), std::invalid_argument);
    CHECK_THROWS_AS(UnitPhysicalProperties(1.0, 150.0, 0.0), std::invalid_argument);
    CHECK_THROWS_AS(UnitPhysicalProperties(std::numeric_limits<double>::infinity(), 1.0, 1.0), std::invalid_argument);
    CHECK_THROWS_AS(UnitPhysicalProperties(1.0, std::numeric_limits<double>::quiet_NaN(), 1.0), std::invalid_argument);
}

TEST_CASE("Unit component references require a positive quantity") {
    const UnitComponentReference reference(DefinitionId("marine_engine"), 2);
    CHECK(reference.componentId().value() == "marine_engine");
    CHECK(reference.quantity() == 2);
    CHECK_THROWS_AS(UnitComponentReference(DefinitionId("marine_engine"), 0), std::invalid_argument);
}

TEST_CASE("Unit definition stores generic metadata and component references") {
    UnitDefinition definition(
        DefinitionId("hms_intrepid"), "HMS Intrepid", "frigate", UnitEnvironment::Water,
        UnitPhysicalProperties(4500000.0, 150.0, 20.0),
        {UnitComponentReference(DefinitionId("marine_engine"), 2)}
    );

    CHECK(definition.id().value() == "hms_intrepid");
    CHECK(definition.name() == "HMS Intrepid");
    CHECK(definition.type() == "frigate");
    CHECK((definition.environment() == UnitEnvironment::Water));
    CHECK(std::string(toString(definition.environment())) == "water");
    REQUIRE(definition.components().size() == 1);
    CHECK(definition.components().front().componentId().value() == "marine_engine");
    CHECK(definition.components().front().quantity() == 2);
}

TEST_CASE("Unit definition rejects empty descriptive fields and invalid environment") {
    const UnitPhysicalProperties physical(1.0, 1.0, 1.0);
    CHECK_THROWS_AS(UnitDefinition(DefinitionId("unit"), "", "frigate", UnitEnvironment::Water, physical, {}), std::invalid_argument);
    CHECK_THROWS_AS(UnitDefinition(DefinitionId("unit"), "Unit", "", UnitEnvironment::Water, physical, {}), std::invalid_argument);
    CHECK_THROWS_AS(UnitDefinition(DefinitionId("unit"), "Unit", "frigate", static_cast<UnitEnvironment>(99), physical, {}), std::invalid_argument);
}

TEST_CASE("Unit environment names are stable data identifiers") {
    CHECK(std::string(toString(UnitEnvironment::Water)) == "water");
    CHECK(std::string(toString(UnitEnvironment::Air)) == "air");
    CHECK(std::string(toString(UnitEnvironment::Land)) == "land");
}

