#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <limits>
#include <stdexcept>
#include <string>
#include <variant>

#include "data/definitions/component_definition.hpp"

using namespace navalsim::data;

TEST_CASE("Component categories have stable data identifiers") {
    CHECK(std::string(toString(ComponentCategory::DamageOnly)) == "damage_only");
    CHECK(std::string(toString(ComponentCategory::Simulation)) == "simulation");
}

TEST_CASE("Radar properties store their static definition values") {
    const RadarProperties properties(120.0);
    CHECK(properties.rangeNm() == doctest::Approx(120.0));
}

TEST_CASE("Radar properties reject non-positive and non-finite values") {
    CHECK_THROWS_AS(RadarProperties(0.0), std::invalid_argument);
    CHECK_THROWS_AS(RadarProperties(-1.0), std::invalid_argument);
    CHECK_THROWS_AS((RadarProperties{std::numeric_limits<double>::infinity()}), std::invalid_argument);
    CHECK_THROWS_AS((RadarProperties{std::numeric_limits<double>::quiet_NaN()}), std::invalid_argument);
}

TEST_CASE("Damage-only components have a type but no simulation properties") {
    const ComponentDefinition engine(
        DefinitionId("marine_engine"),
        "Marine Engine",
        ComponentCategory::DamageOnly,
        "engine"
    );

    CHECK(engine.id().value() == "marine_engine");
    CHECK(engine.name() == "Marine Engine");
    CHECK((engine.category() == ComponentCategory::DamageOnly));
    CHECK(engine.type() == "engine");
    CHECK_FALSE(engine.properties().has_value());

    const ComponentDefinition propeller(
        DefinitionId("propeller"),
        "Propeller",
        ComponentCategory::DamageOnly,
        "propeller"
    );
    CHECK(propeller.type() == "propeller");
}

TEST_CASE("Simulation components can carry type-specific static properties") {
    const ComponentDefinition radar(
        DefinitionId("radar_search"),
        "Search Radar",
        ComponentCategory::Simulation,
        "radar",
        ComponentProperties{RadarProperties(120.0)}
    );

    CHECK((radar.category() == ComponentCategory::Simulation));
    CHECK(radar.type() == "radar");
    REQUIRE(radar.properties().has_value());
    REQUIRE(std::holds_alternative<RadarProperties>(*radar.properties()));
    CHECK(std::get<RadarProperties>(*radar.properties()).rangeNm() == doctest::Approx(120.0));
}

TEST_CASE("Simulation component types without agreed properties remain representable") {
    const ComponentDefinition launcher(
        DefinitionId("sam_launcher"),
        "Surface-to-Air Missile Launcher",
        ComponentCategory::Simulation,
        "sam_launcher"
    );

    CHECK((launcher.category() == ComponentCategory::Simulation));
    CHECK(launcher.type() == "sam_launcher");
    CHECK_FALSE(launcher.properties().has_value());
}

TEST_CASE("Component definition rejects invalid metadata and category-property combinations") {
    CHECK_THROWS_AS(
        ComponentDefinition(DefinitionId("engine"), "", ComponentCategory::DamageOnly, "engine"),
        std::invalid_argument
    );
    CHECK_THROWS_AS(
        ComponentDefinition(DefinitionId("engine"), "Engine", ComponentCategory::DamageOnly, ""),
        std::invalid_argument
    );
    CHECK_THROWS_AS(
        ComponentDefinition(DefinitionId("engine"), "Engine", static_cast<ComponentCategory>(99), "engine"),
        std::invalid_argument
    );
    CHECK_THROWS_AS(
        ComponentDefinition(
            DefinitionId("engine"),
            "Engine",
            ComponentCategory::DamageOnly,
            "engine",
            ComponentProperties{RadarProperties(20.0)}
        ),
        std::invalid_argument
    );
    CHECK_THROWS_AS(
        ComponentDefinition(DefinitionId("radar"), "Radar", ComponentCategory::DamageOnly, "radar"),
        std::invalid_argument
    );
    CHECK_THROWS_AS(
        ComponentDefinition(DefinitionId("radar"), "Radar", ComponentCategory::Simulation, "radar"),
        std::invalid_argument
    );
    CHECK_THROWS_AS(
        ComponentDefinition(
            DefinitionId("antenna"),
            "Antenna",
            ComponentCategory::Simulation,
            "antenna",
            ComponentProperties{RadarProperties(20.0)}
        ),
        std::invalid_argument
    );
}
