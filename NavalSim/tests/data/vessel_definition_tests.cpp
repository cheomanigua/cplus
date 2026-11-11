
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <limits>
#include <stdexcept>
#include <string>

#include "data/definitions/vessel_definition.hpp"

using namespace navalsim::data;
using namespace navalsim::geography;

TEST_CASE("Definition ID stores a non-empty identifier") {
    const DefinitionId id("destroyer.ddg51");
    CHECK(id.value() == "destroyer.ddg51");
}

TEST_CASE("Definition ID rejects an empty identifier") {
    CHECK_THROWS_AS(DefinitionId(""), std::invalid_argument);
}

TEST_CASE("Vessel dimensions store their measurements") {
    const VesselDimensions dimensions(
        Distance(155.0),
        Distance(20.0),
        Distance(9.5)
    );

    CHECK(dimensions.length().meters() == doctest::Approx(155.0));
    CHECK(dimensions.beam().meters() == doctest::Approx(20.0));
    CHECK(dimensions.draft().meters() == doctest::Approx(9.5));
}

TEST_CASE("Vessel dimensions reject non-positive measurements") {
    CHECK_THROWS_AS((VesselDimensions(Distance(0.0), Distance(20.0), Distance(9.5))), std::invalid_argument);
    CHECK_THROWS_AS((VesselDimensions(Distance(155.0), Distance(0.0), Distance(9.5))), std::invalid_argument);
    CHECK_THROWS_AS((VesselDimensions(Distance(155.0), Distance(20.0), Distance(0.0))), std::invalid_argument);
}

TEST_CASE("Vessel propulsion stores maximum speed") {
    const VesselPropulsion propulsion(30.0);
    CHECK(propulsion.maximumSpeedMetersPerSecond() == doctest::Approx(30.0));
}

TEST_CASE("Vessel propulsion rejects invalid maximum speeds") {
    CHECK_THROWS_AS(VesselPropulsion(0.0), std::invalid_argument);
    CHECK_THROWS_AS(VesselPropulsion(-1.0), std::invalid_argument);
    CHECK_THROWS_AS(VesselPropulsion(std::numeric_limits<double>::infinity()), std::invalid_argument);
    CHECK_THROWS_AS(VesselPropulsion(std::numeric_limits<double>::quiet_NaN()), std::invalid_argument);
}

TEST_CASE("Vessel definition stores its typed properties") {
    const VesselDefinition definition(
        DefinitionId("destroyer.ddg51"),
        "Arleigh Burke-class Destroyer",
        VesselDimensions(Distance(155.0), Distance(20.0), Distance(9.5)),
        VesselPropulsion(30.0)
    );

    CHECK(definition.id().value() == "destroyer.ddg51");
    CHECK(definition.name() == "Arleigh Burke-class Destroyer");
    CHECK(definition.dimensions().length().meters() == doctest::Approx(155.0));
    CHECK(definition.propulsion().maximumSpeedMetersPerSecond() == doctest::Approx(30.0));
}

TEST_CASE("Vessel definition rejects an empty name") {
    CHECK_THROWS_AS(
        (VesselDefinition(
            DefinitionId("destroyer.ddg51"),
            "",
            VesselDimensions(Distance(155.0), Distance(20.0), Distance(9.5)),
            VesselPropulsion(30.0)
        )),
        std::invalid_argument
    );
}

