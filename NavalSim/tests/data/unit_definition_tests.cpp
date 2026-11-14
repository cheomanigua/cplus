#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "data/definitions/unit_definition.hpp"

using namespace navalsim::data;

namespace {

UnitDefinition makeUnit() {
    return UnitDefinition(
        DefinitionId("hms_intrepid"),
        "HMS Intrepid",
        "frigate",
        UnitEnvironment::Water,
        30.0,
        FuelProperties{120000.0, 20.0, 1500.0},
        {UnitComponentReference(DefinitionId("marine_engine"), 2)}
    );
}

} // namespace

TEST_CASE("Unit component references require a positive quantity") {
    const UnitComponentReference reference(DefinitionId("marine_engine"), 2);
    CHECK(reference.componentId().value() == "marine_engine");
    CHECK(reference.quantity() == 2);
    CHECK_THROWS_AS(
        UnitComponentReference(DefinitionId("marine_engine"), 0),
        std::invalid_argument
    );
}

TEST_CASE("Unit definition stores identity, movement, optional fuel, and component references") {
    const auto definition = makeUnit();

    CHECK(definition.id().value() == "hms_intrepid");
    CHECK(definition.name() == "HMS Intrepid");
    CHECK(definition.type() == "frigate");
    CHECK((definition.environment() == UnitEnvironment::Water));
    CHECK(std::string(toString(definition.environment())) == "water");
    CHECK(definition.maxSpeedKnots() == doctest::Approx(30.0));

    REQUIRE(definition.fuel().has_value());
    CHECK(definition.fuel()->capacity == doctest::Approx(120000.0));
    CHECK(definition.fuel()->referenceSpeedKnots == doctest::Approx(20.0));
    CHECK(definition.fuel()->consumptionPerHour == doctest::Approx(1500.0));

    REQUIRE(definition.components().size() == 1);
    CHECK(definition.components().front().componentId().value() == "marine_engine");
    CHECK(definition.components().front().quantity() == 2);
}

TEST_CASE("Unit definition supports units without propulsion fuel") {
    const UnitDefinition definition(
        DefinitionId("coastal_radar_station"),
        "Coastal Radar Station",
        "radar_station",
        UnitEnvironment::Land,
        0.0,
        std::nullopt,
        {}
    );

    CHECK(definition.maxSpeedKnots() == doctest::Approx(0.0));
    CHECK_FALSE(definition.fuel().has_value());
}

TEST_CASE("Unit definition accepts zero maximum speed for stationary units") {
    const UnitDefinition definition(
        DefinitionId("station"),
        "Station",
        "coastal_station",
        UnitEnvironment::Land,
        0.0,
        std::nullopt,
        {}
    );

    CHECK(definition.maxSpeedKnots() == doctest::Approx(0.0));
}

TEST_CASE("Unit definition rejects empty descriptive fields and invalid environment") {
    const auto make = [](std::string name, std::string type, UnitEnvironment environment) {
        return UnitDefinition(
            DefinitionId("unit"),
            std::move(name),
            std::move(type),
            environment,
            1.0,
            std::nullopt,
            {}
        );
    };

    CHECK_THROWS_AS(make("", "frigate", UnitEnvironment::Water), std::invalid_argument);
    CHECK_THROWS_AS(make("Unit", "", UnitEnvironment::Water), std::invalid_argument);
    CHECK_THROWS_AS(
        make("Unit", "frigate", static_cast<UnitEnvironment>(99)),
        std::invalid_argument
    );
}

TEST_CASE("Unit definition rejects negative and non-finite maximum speeds") {
    const double infinity = std::numeric_limits<double>::infinity();
    const double nan = std::numeric_limits<double>::quiet_NaN();

    CHECK_THROWS_AS(
        UnitDefinition(DefinitionId("u"), "Unit", "ship", UnitEnvironment::Water,
            -1.0, std::nullopt, {}),
        std::invalid_argument
    );
    CHECK_THROWS_AS(
        UnitDefinition(DefinitionId("u"), "Unit", "ship", UnitEnvironment::Water,
            infinity, std::nullopt, {}),
        std::invalid_argument
    );
    CHECK_THROWS_AS(
        UnitDefinition(DefinitionId("u"), "Unit", "ship", UnitEnvironment::Water,
            nan, std::nullopt, {}),
        std::invalid_argument
    );
}

TEST_CASE("Unit definition rejects invalid fuel properties") {
    const double infinity = std::numeric_limits<double>::infinity();
    const double nan = std::numeric_limits<double>::quiet_NaN();

    const auto make = [](FuelProperties fuel) {
        return UnitDefinition(
            DefinitionId("u"),
            "Unit",
            "ship",
            UnitEnvironment::Water,
            10.0,
            fuel,
            {}
        );
    };

    CHECK_THROWS_AS(make({0.0, 1.0, 1.0}), std::invalid_argument);
    CHECK_THROWS_AS(make({-1.0, 1.0, 1.0}), std::invalid_argument);
    CHECK_THROWS_AS(make({infinity, 1.0, 1.0}), std::invalid_argument);
    CHECK_THROWS_AS(make({1.0, 0.0, 1.0}), std::invalid_argument);
    CHECK_THROWS_AS(make({1.0, -1.0, 1.0}), std::invalid_argument);
    CHECK_THROWS_AS(make({1.0, nan, 1.0}), std::invalid_argument);
    CHECK_THROWS_AS(make({1.0, 1.0, 0.0}), std::invalid_argument);
    CHECK_THROWS_AS(make({1.0, 1.0, -1.0}), std::invalid_argument);
    CHECK_THROWS_AS(make({1.0, 1.0, infinity}), std::invalid_argument);
}

TEST_CASE("Unit environment names are stable data identifiers") {
    CHECK(std::string(toString(UnitEnvironment::Water)) == "water");
    CHECK(std::string(toString(UnitEnvironment::Air)) == "air");
    CHECK(std::string(toString(UnitEnvironment::Land)) == "land");
}
