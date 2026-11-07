#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <doctest/doctest.h>

#include <flecs.h>

#include <limits>
#include <stdexcept>

#include "core/geography/geographic_types.hpp"
#include "domains/movement/movement_components.hpp"
#include "world/components/tags/entity_type_tags.hpp"

using namespace navalsim::geography;
using namespace navalsim::movement;


TEST_CASE("Movement components can be attached to a vessel entity")
{
    flecs::world world;

    const auto vessel = world.entity("Test Vessel")
        .add<navalsim::world::Vessel>()
        .set<GeographicPosition>({
            Latitude(51.5),
            Longitude(-0.1),
            Elevation(0.0)
        })
        .set<Heading>(Heading(90.0))
        .set<Speed>(Speed(12.0));

    CHECK(vessel.has<navalsim::world::Vessel>());
    CHECK(vessel.has<GeographicPosition>());
    CHECK(vessel.has<Heading>());
    CHECK(vessel.has<Speed>());

    const auto& position = vessel.get<GeographicPosition>();
    const auto& heading = vessel.get<Heading>();
    const auto& speed = vessel.get<Speed>();

    CHECK(
        position.latitude().degrees() ==
        doctest::Approx(51.5)
    );

    CHECK(
        position.longitude().degrees() ==
        doctest::Approx(-0.1)
    );

    CHECK(
        position.elevation().meters() ==
        doctest::Approx(0.0)
    );

    CHECK(heading.degrees() == doctest::Approx(90.0));

    CHECK(
        speed.metersPerSecond() ==
        doctest::Approx(12.0)
    );
}


TEST_CASE("Heading normalizes its direction")
{
    CHECK(Heading(450.0).degrees() == doctest::Approx(90.0));
    CHECK(Heading(-90.0).degrees() == doctest::Approx(270.0));
}


TEST_CASE("Speed rejects invalid values")
{
    CHECK_THROWS_AS(
        Speed(-1.0),
        std::invalid_argument
    );

    CHECK_THROWS_AS(
        Speed(std::numeric_limits<double>::infinity()),
        std::invalid_argument
    );

    CHECK_THROWS_AS(
        Speed(std::numeric_limits<double>::quiet_NaN()),
        std::invalid_argument
    );
}


TEST_CASE("Heading can be changed")
{
    navalsim::movement::Heading heading(90.0);

    heading.setDegrees(180.0);

    CHECK(heading.degrees() == doctest::Approx(180.0));
}

TEST_CASE("Heading changes are normalized")
{
    navalsim::movement::Heading heading(350.0);

    heading.setDegrees(370.0);

    CHECK(heading.degrees() == doctest::Approx(10.0));

    heading.setDegrees(-10.0);

    CHECK(heading.degrees() == doctest::Approx(350.0));
}

TEST_CASE("Turn rate stores degrees per second")
{
    navalsim::movement::TurnRate turnRate(2.5);

    CHECK(
        turnRate.degreesPerSecond()
        == doctest::Approx(2.5)
    );
}

TEST_CASE("Turn rate defaults to zero")
{
    navalsim::movement::TurnRate turnRate;

    CHECK(
        turnRate.degreesPerSecond()
        == doctest::Approx(0.0)
    );
}

TEST_CASE("Turn rate rejects negative values")
{
    CHECK_THROWS_AS(
        navalsim::movement::TurnRate(-1.0),
        std::invalid_argument
    );
}

TEST_CASE("Turn rate rejects non-finite values")
{
    CHECK_THROWS_AS(
        navalsim::movement::TurnRate(
            std::numeric_limits<double>::infinity()
        ),
        std::invalid_argument
    );

    CHECK_THROWS_AS(
        navalsim::movement::TurnRate(
            std::numeric_limits<double>::quiet_NaN()
        ),
        std::invalid_argument
    );
}

