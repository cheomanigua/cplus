#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>

#include "data/definitions/component_definition.hpp"

using namespace navalsim::data;

TEST_CASE("AAA properties store their capability values") {
    const AaaProperties properties(4500.0, 20.0);
    CHECK(properties.ratePerMinute() == doctest::Approx(4500.0));
    CHECK(properties.caliberMm() == doctest::Approx(20.0));
}

TEST_CASE("SAM properties store their capability values") {
    const SamProperties properties(80.0, 1200.0, 25.0);
    CHECK(properties.rangeNm() == doctest::Approx(80.0));
    CHECK(properties.speedMetersPerSecond() == doctest::Approx(1200.0));
    CHECK(properties.warheadKg() == doctest::Approx(25.0));
}

TEST_CASE("Engine and radar properties store their capability values") {
    CHECK(EngineProperties(250000.0).thrustN() == doctest::Approx(250000.0));
    CHECK(RadarProperties(120.0).rangeNm() == doctest::Approx(120.0));
}

TEST_CASE("Component capability properties reject invalid values") {
    const double infinity = std::numeric_limits<double>::infinity();
    const double nan = std::numeric_limits<double>::quiet_NaN();

    CHECK_THROWS_AS(AaaProperties(0.0, 20.0), std::invalid_argument);
    CHECK_THROWS_AS(AaaProperties(1.0, -1.0), std::invalid_argument);
    CHECK_THROWS_AS(AaaProperties(infinity, 20.0), std::invalid_argument);
    CHECK_THROWS_AS(SamProperties(0.0, 1.0, 1.0), std::invalid_argument);
    CHECK_THROWS_AS(SamProperties(1.0, infinity, 1.0), std::invalid_argument);
    CHECK_THROWS_AS(SamProperties(1.0, 1.0, nan), std::invalid_argument);
    CHECK_THROWS_AS(EngineProperties(0.0), std::invalid_argument);
    CHECK_THROWS_AS((EngineProperties{infinity}), std::invalid_argument);
    CHECK_THROWS_AS(RadarProperties(-1.0), std::invalid_argument);
    CHECK_THROWS_AS((RadarProperties{nan}), std::invalid_argument);
}

TEST_CASE("Component definition derives its type from its typed properties") {
    const ComponentDefinition aaa(DefinitionId("aaa_phalanx"), "Phalanx CIWS", AaaProperties(4500.0, 20.0));
    const ComponentDefinition sam(DefinitionId("sam_example"), "Example SAM", SamProperties(80.0, 1200.0, 25.0));
    const ComponentDefinition engine(DefinitionId("marine_engine"), "Marine Engine", EngineProperties(250000.0));
    const ComponentDefinition radar(DefinitionId("radar_search"), "Search Radar", RadarProperties(120.0));

    CHECK((aaa.type() == ComponentType::Aaa));
    CHECK((sam.type() == ComponentType::Sam));
    CHECK((engine.type() == ComponentType::Engine));
    CHECK((radar.type() == ComponentType::Radar));

    CHECK(std::string(toString(aaa.type())) == "aaa");
    CHECK(std::string(toString(sam.type())) == "sam");
    CHECK(std::string(toString(engine.type())) == "engine");
    CHECK(std::string(toString(radar.type())) == "radar");

    CHECK(aaa.id().value() == "aaa_phalanx");
    CHECK(aaa.name() == "Phalanx CIWS");
}

TEST_CASE("Component definition rejects an empty name") {
    CHECK_THROWS_AS(ComponentDefinition(DefinitionId("radar"), "", RadarProperties(120.0)), std::invalid_argument);
}

TEST_CASE("Component properties are represented by a closed typed variant") {
    static_assert(std::variant_size_v<ComponentProperties> == 4);
    CHECK(std::holds_alternative<AaaProperties>(ComponentProperties{AaaProperties(30.0, 30.0)}));
}

