#pragma once

#include "core/geography/geographic_types.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>
#include <stdexcept>

namespace navalsim::geography
{

// Mean radius of the Earth in metres.
inline constexpr double kMeanEarthRadiusMeters = 6371008.8;

// Calculates the horizontal surface distance between two positions.
//
// Uses the Haversine formula and assumes a spherical Earth.
// Elevation is intentionally ignored.
inline Distance horizontalDistance(
    const GeographicPosition& start,
    const GeographicPosition& destination)
{
    constexpr double degreesToRadians =
        std::numbers::pi / 180.0;

    const double latitude1 =
        start.latitude().degrees() * degreesToRadians;

    const double latitude2 =
        destination.latitude().degrees() * degreesToRadians;

    const double latitudeDifference = latitude2 - latitude1;

    const double longitudeDifference =
        (destination.longitude().degrees() -
         start.longitude().degrees()) * degreesToRadians;

    const double sinHalfLatitude =
        std::sin(latitudeDifference / 2.0);

    const double sinHalfLongitude =
        std::sin(longitudeDifference / 2.0);

    const double haversine =
        sinHalfLatitude * sinHalfLatitude +
        std::cos(latitude1) *
        std::cos(latitude2) *
        sinHalfLongitude * sinHalfLongitude;

    // Protect against floating-point rounding outside [0, 1].
    const double clampedHaversine =
        std::clamp(haversine, 0.0, 1.0);

    const double centralAngle =
        2.0 * std::atan2(
            std::sqrt(clampedHaversine),
            std::sqrt(1.0 - clampedHaversine)
        );

    return Distance(
        kMeanEarthRadiusMeters * centralAngle
    );
}


// Calculates the initial bearing from one geographic position
// toward another.
//
// The bearing is measured clockwise from true north and normalized
// to the range [0, 360).
//
// Throws std::invalid_argument if the direction is undefined,
// such as for identical or antipodal positions.
//
// Elevation is intentionally ignored.
inline Bearing initialBearing(
    const GeographicPosition& start,
    const GeographicPosition& destination)
{
    constexpr double degreesToRadians =
        std::numbers::pi / 180.0;

    constexpr double undefinedBearingThreshold = 1e-12;

    const double latitude1 =
        start.latitude().degrees() * degreesToRadians;

    const double latitude2 =
        destination.latitude().degrees() * degreesToRadians;

    const double longitudeDifference =
        (destination.longitude().degrees() -
         start.longitude().degrees()) * degreesToRadians;

    const double y =
        std::sin(longitudeDifference) * std::cos(latitude2);

    const double x =
        std::cos(latitude1) * std::sin(latitude2) -
        std::sin(latitude1) * std::cos(latitude2) *
        std::cos(longitudeDifference);

    // At identical or antipodal positions, the initial direction
    // is mathematically undefined.
    if (std::hypot(x, y) <= undefinedBearingThreshold)
    {
        throw std::invalid_argument(
            "Initial bearing is undefined for these positions."
        );
    }

    const double bearingRadians = std::atan2(y, x);
    const double bearingDegrees =
        bearingRadians / degreesToRadians;

    return Bearing(bearingDegrees);
}


// Calculates a destination position given a starting position,
// initial bearing and horizontal travel distance.
//
// Uses spherical Earth calculations.
// The starting elevation is preserved.
//
// Longitude is normalized to the range [-180, 180).
inline GeographicPosition destinationPosition(
    const GeographicPosition& start,
    Bearing bearing,
    Distance distance)
{
    constexpr double degreesToRadians =
        std::numbers::pi / 180.0;

    constexpr double radiansToDegrees =
        180.0 / std::numbers::pi;

    const double latitude1 =
        start.latitude().degrees() * degreesToRadians;

    const double longitude1 =
        start.longitude().degrees() * degreesToRadians;

    const double bearingRadians =
        bearing.degrees() * degreesToRadians;

    const double angularDistance =
        distance.meters() / kMeanEarthRadiusMeters;

    // Avoid unnecessary floating-point calculations when
    // the position is not moving.
    if (distance.meters() == 0.0)
    {
        return GeographicPosition(
            start.latitude(),
            start.longitude(),
            start.elevation()
        );
    }

    // Calculate destination latitude.
    const double sinLatitude2 =
        std::sin(latitude1) * std::cos(angularDistance) +
        std::cos(latitude1) * std::sin(angularDistance) *
        std::cos(bearingRadians);

    const double latitude2 =
        std::asin(std::clamp(sinLatitude2, -1.0, 1.0));

    // Calculate destination longitude.
    const double longitude2 =
        longitude1 +
        std::atan2(
            std::sin(bearingRadians) *
                std::sin(angularDistance) *
                std::cos(latitude1),

            std::cos(angularDistance) -
                std::sin(latitude1) * std::sin(latitude2)
        );

    // Convert longitude to degrees.
    double longitudeDegrees =
        longitude2 * radiansToDegrees;

    // Normalize longitude to [-180, 180).
    longitudeDegrees = std::fmod(
        longitudeDegrees + 180.0,
        360.0
    );

    if (longitudeDegrees < 0.0)
    {
        longitudeDegrees += 360.0;
    }

    longitudeDegrees -= 180.0;

    return GeographicPosition(
        Latitude(latitude2 * radiansToDegrees),
        Longitude(longitudeDegrees),
        start.elevation()
    );
}

} // namespace navalsim::geography
