#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "core/geography/geographic_types.hpp"

#include <limits>
#include <stdexcept>

using namespace navalsim::geography;

// --------------------------------------------------
// Latitude
// --------------------------------------------------

TEST_CASE("Latitude accepts valid values")
{
    CHECK(Latitude(0.0).degrees() == doctest::Approx(0.0));
    CHECK(Latitude(90.0).degrees() == doctest::Approx(90.0));
    CHECK(Latitude(-90.0).degrees() == doctest::Approx(-90.0));
}

TEST_CASE("Latitude rejects invalid values")
{
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double infinity = std::numeric_limits<double>::infinity();

    CHECK_THROWS_AS((Latitude{90.1}), std::invalid_argument);
    CHECK_THROWS_AS((Latitude{-90.1}), std::invalid_argument);
    CHECK_THROWS_AS((Latitude{nan}), std::invalid_argument);
    CHECK_THROWS_AS((Latitude{infinity}), std::invalid_argument);
}

// --------------------------------------------------
// Longitude
// --------------------------------------------------

TEST_CASE("Longitude accepts valid values")
{
    CHECK(Longitude(0.0).degrees() == doctest::Approx(0.0));
    CHECK(Longitude(180.0).degrees() == doctest::Approx(180.0));
    CHECK(Longitude(-180.0).degrees() == doctest::Approx(-180.0));
}

TEST_CASE("Longitude rejects invalid values")
{
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double infinity = std::numeric_limits<double>::infinity();

    CHECK_THROWS_AS((Longitude{180.1}), std::invalid_argument);
    CHECK_THROWS_AS((Longitude{-180.1}), std::invalid_argument);
    CHECK_THROWS_AS((Longitude{nan}), std::invalid_argument);
    CHECK_THROWS_AS((Longitude{infinity}), std::invalid_argument);
}

// --------------------------------------------------
// Elevation
// --------------------------------------------------

TEST_CASE("Elevation accepts positive, zero and negative values")
{
    CHECK(Elevation(5000.0).meters() == doctest::Approx(5000.0));
    CHECK(Elevation(0.0).meters() == doctest::Approx(0.0));
    CHECK(Elevation(-200.0).meters() == doctest::Approx(-200.0));
}

TEST_CASE("Elevation rejects non-finite values")
{
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double infinity = std::numeric_limits<double>::infinity();

    CHECK_THROWS_AS((Elevation{nan}), std::invalid_argument);
    CHECK_THROWS_AS((Elevation{infinity}), std::invalid_argument);
    CHECK_THROWS_AS((Elevation{-infinity}), std::invalid_argument);
}

// --------------------------------------------------
// GeographicPosition
// --------------------------------------------------

TEST_CASE("GeographicPosition stores three-dimensional coordinates")
{
    const GeographicPosition position(
        Latitude(42.0),
        Longitude(3.0),
        Elevation(-200.0)
    );

    CHECK(position.latitude().degrees() == doctest::Approx(42.0));
    CHECK(position.longitude().degrees() == doctest::Approx(3.0));
    CHECK(position.elevation().meters() == doctest::Approx(-200.0));
}

TEST_CASE("GeographicPosition defaults to sea level")
{
    const GeographicPosition position(
        Latitude(42.0),
        Longitude(3.0)
    );

    CHECK(position.elevation().meters() == doctest::Approx(0.0));
}

// --------------------------------------------------
// Distance
// --------------------------------------------------

TEST_CASE("Distance accepts valid values")
{
    CHECK(Distance(0.0).meters() == doctest::Approx(0.0));
    CHECK(Distance(1000.0).meters() == doctest::Approx(1000.0));
}

TEST_CASE("Distance rejects negative and non-finite values")
{
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double infinity = std::numeric_limits<double>::infinity();

    CHECK_THROWS_AS((Distance{-1.0}), std::invalid_argument);
    CHECK_THROWS_AS((Distance{nan}), std::invalid_argument);
    CHECK_THROWS_AS((Distance{infinity}), std::invalid_argument);
}

// --------------------------------------------------
// Bearing
// --------------------------------------------------

TEST_CASE("Bearing normalizes angles")
{
    CHECK(Bearing(0.0).degrees() == doctest::Approx(0.0));
    CHECK(Bearing(90.0).degrees() == doctest::Approx(90.0));
    CHECK(Bearing(360.0).degrees() == doctest::Approx(0.0));
    CHECK(Bearing(370.0).degrees() == doctest::Approx(10.0));
    CHECK(Bearing(-10.0).degrees() == doctest::Approx(350.0));
    CHECK(Bearing(-360.0).degrees() == doctest::Approx(0.0));
}

TEST_CASE("Bearing rejects non-finite values")
{
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double infinity = std::numeric_limits<double>::infinity();

    CHECK_THROWS_AS((Bearing{nan}), std::invalid_argument);
    CHECK_THROWS_AS((Bearing{infinity}), std::invalid_argument);
}
