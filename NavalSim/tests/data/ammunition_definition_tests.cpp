#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <limits>
#include <stdexcept>
#include <string>

#include "data/definitions/ammunition_definition.hpp"

using namespace navalsim::data;

TEST_CASE("Ammunition definition stores reusable static characteristics") {
    const AmmunitionDefinition definition(
        DefinitionId("aim9_sidewinder"),
        "AIM-9 Sidewinder",
        18.0,
        850.0,
        true,
        80.0
    );

    CHECK(definition.id().value() == "aim9_sidewinder");
    CHECK(definition.name() == "AIM-9 Sidewinder");
    CHECK(definition.range() == doctest::Approx(18.0));
    CHECK(definition.speed() == doctest::Approx(850.0));
    CHECK(definition.isGuided());
    CHECK(definition.damage() == doctest::Approx(80.0));
}

TEST_CASE("Ammunition definition rejects empty names") {
    CHECK_THROWS_AS(
        AmmunitionDefinition(DefinitionId("ammo"), "", 1.0, 1.0, true, 1.0),
        std::invalid_argument
    );
}

TEST_CASE("Ammunition definition rejects non-positive and non-finite characteristics") {
    const double infinity = std::numeric_limits<double>::infinity();
    const double nan = std::numeric_limits<double>::quiet_NaN();

    CHECK_THROWS_AS(AmmunitionDefinition(DefinitionId("a"), "A", 0.0, 1.0, true, 1.0), std::invalid_argument);
    CHECK_THROWS_AS(AmmunitionDefinition(DefinitionId("a"), "A", 1.0, -1.0, true, 1.0), std::invalid_argument);
    CHECK_THROWS_AS(AmmunitionDefinition(DefinitionId("a"), "A", 1.0, 1.0, true, infinity), std::invalid_argument);
    CHECK_THROWS_AS(AmmunitionDefinition(DefinitionId("a"), "A", nan, 1.0, false, 1.0), std::invalid_argument);
}
