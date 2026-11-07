#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <cmath>
#include <numbers>

#include "core/geography/geographic_calculations.hpp"

using namespace navalsim::geography;

// --------------------------------------------------
// Horizontal distance
// --------------------------------------------------

TEST_CASE("Horizontal distance between identical positions is zero")
{
    const GeographicPosition position(
        Latitude(42.0),
        Longitude(3.0)
    );

    const Distance distance =
        horizontalDistance(position, position);

    CHECK(distance.meters() == doctest::Approx(0.0));
}

TEST_CASE("Horizontal distance along the equator is correct")
{
    const GeographicPosition start(
        Latitude(0.0),
        Longitude(0.0)
    );

    const GeographicPosition destination(
        Latitude(0.0),
        Longitude(1.0)
    );

    const Distance distance =
        horizontalDistance(start, destination);

    CHECK(
        distance.meters() ==
        doctest::Approx(111195.0802).epsilon(0.000001)
    );
}

TEST_CASE("Horizontal distance is symmetric")
{
    const GeographicPosition first(
        Latitude(42.0),
        Longitude(3.0)
    );

    const GeographicPosition second(
        Latitude(48.0),
        Longitude(12.0)
    );

    const Distance forward =
        horizontalDistance(first, second);

    const Distance backward =
        horizontalDistance(second, first);

    CHECK(
        forward.meters() ==
        doctest::Approx(backward.meters())
    );
}

TEST_CASE("Horizontal distance handles the antimeridian")
{
    const GeographicPosition start(
        Latitude(0.0),
        Longitude(179.0)
    );

    const GeographicPosition destination(
        Latitude(0.0),
        Longitude(-179.0)
    );

    const Distance distance =
        horizontalDistance(start, destination);

    CHECK(
        distance.meters() ==
        doctest::Approx(222390.1605).epsilon(0.000001)
    );
}

TEST_CASE("Horizontal distance handles opposite points on the equator")
{
    const GeographicPosition start(
        Latitude(0.0),
        Longitude(0.0)
    );

    const GeographicPosition destination(
        Latitude(0.0),
        Longitude(180.0)
    );

    const Distance distance =
        horizontalDistance(start, destination);

    const double expectedDistance =
        std::numbers::pi * kMeanEarthRadiusMeters;

    CHECK(
        distance.meters() ==
        doctest::Approx(expectedDistance)
    );
}

TEST_CASE("Horizontal distance ignores elevation differences")
{
    const GeographicPosition seaLevel(
        Latitude(42.0),
        Longitude(3.0),
        Elevation(0.0)
    );

    const GeographicPosition aboveSeaLevel(
        Latitude(42.0),
        Longitude(3.0),
        Elevation(5000.0)
    );

    const Distance distance =
        horizontalDistance(seaLevel, aboveSeaLevel);

    CHECK(distance.meters() == doctest::Approx(0.0));
}

TEST_CASE("Horizontal distance between the poles is half the Earth circumference")
{
    const GeographicPosition northPole(
        Latitude(90.0),
        Longitude(0.0)
    );

    const GeographicPosition southPole(
        Latitude(-90.0),
        Longitude(0.0)
    );

    const Distance distance =
        horizontalDistance(northPole, southPole);

    const double expectedDistance =
        std::numbers::pi * kMeanEarthRadiusMeters;

    CHECK(
        distance.meters() ==
        doctest::Approx(expectedDistance)
    );
}

// --------------------------------------------------
// Initial bearing
// --------------------------------------------------

TEST_CASE("Initial bearing points north")
{
    const GeographicPosition start(
        Latitude(0.0),
        Longitude(0.0)
    );

    const GeographicPosition destination(
        Latitude(1.0),
        Longitude(0.0)
    );

    const Bearing bearing =
        initialBearing(start, destination);

    CHECK(bearing.degrees() == doctest::Approx(0.0));
}

TEST_CASE("Initial bearing points east")
{
    const GeographicPosition start(
        Latitude(0.0),
        Longitude(0.0)
    );

    const GeographicPosition destination(
        Latitude(0.0),
        Longitude(1.0)
    );

    const Bearing bearing =
        initialBearing(start, destination);

    CHECK(bearing.degrees() == doctest::Approx(90.0));
}

TEST_CASE("Initial bearing points south")
{
    const GeographicPosition start(
        Latitude(0.0),
        Longitude(0.0)
    );

    const GeographicPosition destination(
        Latitude(-1.0),
        Longitude(0.0)
    );

    const Bearing bearing =
        initialBearing(start, destination);

    CHECK(bearing.degrees() == doctest::Approx(180.0));
}

TEST_CASE("Initial bearing points west")
{
    const GeographicPosition start(
        Latitude(0.0),
        Longitude(0.0)
    );

    const GeographicPosition destination(
        Latitude(0.0),
        Longitude(-1.0)
    );

    const Bearing bearing =
        initialBearing(start, destination);

    CHECK(bearing.degrees() == doctest::Approx(270.0));
}

TEST_CASE("Initial bearing handles the antimeridian")
{
    const GeographicPosition start(
        Latitude(0.0),
        Longitude(179.0)
    );

    const GeographicPosition destination(
        Latitude(0.0),
        Longitude(-179.0)
    );

    const Bearing bearing =
        initialBearing(start, destination);

    CHECK(bearing.degrees() == doctest::Approx(90.0));
}

TEST_CASE("Initial bearing is normalized")
{
    const GeographicPosition start(
        Latitude(0.0),
        Longitude(0.0)
    );

    const GeographicPosition destination(
        Latitude(1.0),
        Longitude(1.0)
    );

    const Bearing bearing =
        initialBearing(start, destination);

    CHECK(bearing.degrees() >= 0.0);
    CHECK(bearing.degrees() < 360.0);
}

TEST_CASE("Initial bearing ignores elevation differences")
{
    const GeographicPosition start(
        Latitude(0.0),
        Longitude(0.0),
        Elevation(0.0)
    );

    const GeographicPosition destination(
        Latitude(1.0),
        Longitude(0.0),
        Elevation(5000.0)
    );

    const Bearing bearing =
        initialBearing(start, destination);

    CHECK(bearing.degrees() == doctest::Approx(0.0));
}

TEST_CASE("Initial bearing rejects identical positions")
{
    const GeographicPosition position(
        Latitude(42.0),
        Longitude(3.0)
    );

    CHECK_THROWS_AS(
        initialBearing(position, position),
        std::invalid_argument
    );
}

TEST_CASE("Initial bearing rejects antipodal positions")
{
    const GeographicPosition start(
        Latitude(0.0),
        Longitude(0.0)
    );

    const GeographicPosition destination(
        Latitude(0.0),
        Longitude(180.0)
    );

    CHECK_THROWS_AS(
        initialBearing(start, destination),
        std::invalid_argument
    );
}



// --------------------------------------------------
// Destination position
// --------------------------------------------------

TEST_CASE("Destination position calculates northward movement")
{
    const GeographicPosition start(
        Latitude(0.0),
        Longitude(0.0)
    );

    const Distance distance(
        kMeanEarthRadiusMeters * std::numbers::pi / 180.0
    );

    const GeographicPosition destination =
        destinationPosition(start, Bearing(0.0), distance);

    CHECK(
        destination.latitude().degrees() ==
        doctest::Approx(1.0)
    );

    CHECK(
        destination.longitude().degrees() ==
        doctest::Approx(0.0)
    );
}

TEST_CASE("Destination position calculates eastward movement")
{
    const GeographicPosition start(
        Latitude(0.0),
        Longitude(0.0)
    );

    const Distance distance(
        kMeanEarthRadiusMeters * std::numbers::pi / 180.0
    );

    const GeographicPosition destination =
        destinationPosition(start, Bearing(90.0), distance);

    CHECK(
        std::abs(destination.latitude().degrees()) < 1e-10
    );

    CHECK(
        destination.longitude().degrees() ==
        doctest::Approx(1.0)
    );
}

TEST_CASE("Destination position calculates southward movement")
{
    const GeographicPosition start(
        Latitude(0.0),
        Longitude(0.0)
    );

    const Distance distance(
        kMeanEarthRadiusMeters * std::numbers::pi / 180.0
    );

    const GeographicPosition destination =
        destinationPosition(start, Bearing(180.0), distance);

    CHECK(
        destination.latitude().degrees() ==
        doctest::Approx(-1.0)
    );

    CHECK(
        destination.longitude().degrees() ==
        doctest::Approx(0.0)
    );
}

TEST_CASE("Destination position calculates westward movement")
{
    const GeographicPosition start(
        Latitude(0.0),
        Longitude(0.0)
    );

    const Distance distance(
        kMeanEarthRadiusMeters * std::numbers::pi / 180.0
    );

    const GeographicPosition destination =
        destinationPosition(start, Bearing(270.0), distance);

    CHECK(
        std::abs(destination.latitude().degrees()) < 1e-10
    );

    CHECK(
        destination.longitude().degrees() ==
        doctest::Approx(-1.0)
    );
}

TEST_CASE("Destination position handles the antimeridian")
{
    const GeographicPosition start(
        Latitude(0.0),
        Longitude(179.0)
    );

    const Distance distance(
        kMeanEarthRadiusMeters * 2.0 * std::numbers::pi / 180.0
    );

    const GeographicPosition destination =
        destinationPosition(start, Bearing(90.0), distance);

    CHECK(
        std::abs(destination.latitude().degrees()) < 1e-10
    );

    CHECK(
        destination.longitude().degrees() ==
        doctest::Approx(-179.0)
    );
}

TEST_CASE("Destination position preserves elevation")
{
    const GeographicPosition start(
        Latitude(42.0),
        Longitude(3.0),
        Elevation(-200.0)
    );

    const GeographicPosition destination =
        destinationPosition(
            start,
            Bearing(90.0),
            Distance(1000.0)
        );

    CHECK(
        destination.elevation().meters() ==
        doctest::Approx(-200.0)
    );
}

TEST_CASE("Destination position preserves the original position at zero distance")
{
    const GeographicPosition start(
        Latitude(42.0),
        Longitude(180.0),
        Elevation(500.0)
    );

    const GeographicPosition destination =
        destinationPosition(
            start,
            Bearing(90.0),
            Distance(0.0)
        );

    CHECK(
        destination.latitude().degrees() ==
        doctest::Approx(42.0)
    );

    CHECK(
        destination.longitude().degrees() ==
        doctest::Approx(180.0)
    );

    CHECK(
        destination.elevation().meters() ==
        doctest::Approx(500.0)
    );
}

TEST_CASE("Destination position produces the requested horizontal distance")
{
    const GeographicPosition start(
        Latitude(42.0),
        Longitude(3.0)
    );

    const Distance requestedDistance(10000.0);

    const GeographicPosition destination =
        destinationPosition(
            start,
            Bearing(45.0),
            requestedDistance
        );

    const Distance actualDistance =
        horizontalDistance(start, destination);

    CHECK(
        actualDistance.meters() ==
        doctest::Approx(requestedDistance.meters()).epsilon(1e-9)
    );
}



 // --------------------------------------------------
 // Geographic calculation edge cases
 // --------------------------------------------------

TEST_CASE("Destination position correctly crosses the North Pole")
{
    const GeographicPosition start(
        Latitude(89.0),
        Longitude(30.0)
    );

    // Travel north by two degrees of arc.
    const Distance distance(
        kMeanEarthRadiusMeters * 2.0 * std::numbers::pi / 180.0
    );

    const GeographicPosition destination =
        destinationPosition(start, Bearing(0.0), distance);

    CHECK(
        destination.latitude().degrees() ==
        doctest::Approx(89.0)
    );

    CHECK(
        destination.longitude().degrees() ==
        doctest::Approx(-150.0)
    );
}

TEST_CASE("Destination position correctly crosses the South Pole")
{
    const GeographicPosition start(
        Latitude(-89.0),
        Longitude(30.0)
    );

    // Travel south by two degrees of arc.
    const Distance distance(
        kMeanEarthRadiusMeters * 2.0 * std::numbers::pi / 180.0
    );

    const GeographicPosition destination =
        destinationPosition(start, Bearing(180.0), distance);

    CHECK(
        destination.latitude().degrees() ==
        doctest::Approx(-89.0)
    );

    CHECK(
        destination.longitude().degrees() ==
        doctest::Approx(-150.0)
    );
}

TEST_CASE("Destination position handles multiple Earth circumferences")
{
    const GeographicPosition start(
        Latitude(0.0),
        Longitude(20.0)
    );

    const double oneDegreeOfArc =
        kMeanEarthRadiusMeters * std::numbers::pi / 180.0;

    const Distance distance(
        (3.0 * 2.0 * std::numbers::pi *
         kMeanEarthRadiusMeters) + oneDegreeOfArc
    );

    const GeographicPosition destination =
        destinationPosition(start, Bearing(90.0), distance);

    CHECK(
        destination.latitude().degrees() ==
        doctest::Approx(0.0).epsilon(1e-9)
    );

    CHECK(
        destination.longitude().degrees() ==
        doctest::Approx(21.0)
    );
}

TEST_CASE("Destination position handles movement very close to a pole")
{
    const GeographicPosition start(
        Latitude(89.999),
        Longitude(45.0)
    );

    const Distance distance(1000.0);

    const GeographicPosition destination =
        destinationPosition(start, Bearing(90.0), distance);

    // The resulting coordinates must remain valid.
    CHECK(destination.latitude().degrees() >= -90.0);
    CHECK(destination.latitude().degrees() <= 90.0);

    CHECK(destination.longitude().degrees() >= -180.0);
    CHECK(destination.longitude().degrees() < 180.0);

    // The calculated position must still be 1,000 metres away.
    const Distance actualDistance =
        horizontalDistance(start, destination);

    CHECK(
        std::abs(actualDistance.meters() - 1000.0) < 1e-5
    );

    // The initial direction must remain east.
    const Bearing actualBearing =
        initialBearing(start, destination);

    CHECK(
        actualBearing.degrees() ==
        doctest::Approx(90.0)
    );
}

TEST_CASE("Destination position agrees with distance and initial bearing calculations")
{
    const GeographicPosition start(
        Latitude(75.0),
        Longitude(170.0)
    );

    const Bearing requestedBearing(35.0);
    const Distance requestedDistance(250000.0);

    const GeographicPosition destination =
        destinationPosition(
            start,
            requestedBearing,
            requestedDistance
        );

    const Distance calculatedDistance =
        horizontalDistance(start, destination);

    const Bearing calculatedBearing =
        initialBearing(start, destination);

    CHECK(
        calculatedDistance.meters() ==
        doctest::Approx(requestedDistance.meters()).epsilon(1e-9)
    );

    CHECK(
        calculatedBearing.degrees() ==
        doctest::Approx(requestedBearing.degrees())
    );
}

