#pragma once

#include <cmath>
#include <stdexcept>

namespace navalsim::geography
{

class Latitude
{
public:
    Latitude() = default;

    explicit Latitude(double degrees)
        : degrees_(degrees)
    {
        if (!std::isfinite(degrees_) ||
            degrees_ < -90.0 ||
            degrees_ > 90.0)
        {
            throw std::invalid_argument(
                "Latitude must be finite and between -90 and 90 degrees."
            );
        }
    }

    double degrees() const noexcept
    {
        return degrees_;
    }

private:
    double degrees_ = 0.0;
};


class Longitude
{
public:
    Longitude() = default;

    explicit Longitude(double degrees)
        : degrees_(degrees)
    {
        if (!std::isfinite(degrees_) ||
            degrees_ < -180.0 ||
            degrees_ > 180.0)
        {
            throw std::invalid_argument(
                "Longitude must be finite and between -180 and 180 degrees."
            );
        }
    }

    double degrees() const noexcept
    {
        return degrees_;
    }

private:
    double degrees_ = 0.0;
};


class Elevation
{
public:
    Elevation() = default;

    explicit Elevation(double meters)
        : meters_(meters)
    {
        if (!std::isfinite(meters_))
        {
            throw std::invalid_argument(
                "Elevation must be finite."
            );
        }
    }

    double meters() const noexcept
    {
        return meters_;
    }

private:
    double meters_ = 0.0;
};


class GeographicPosition
{
public:
    GeographicPosition() = default;

    GeographicPosition(
        Latitude latitude,
        Longitude longitude,
        Elevation elevation = Elevation{}
    )
        : latitude_(latitude),
          longitude_(longitude),
          elevation_(elevation)
    {
    }

    Latitude latitude() const noexcept
    {
        return latitude_;
    }

    Longitude longitude() const noexcept
    {
        return longitude_;
    }

    Elevation elevation() const noexcept
    {
        return elevation_;
    }

private:
    Latitude latitude_;
    Longitude longitude_;
    Elevation elevation_;
};


class Distance
{
public:
    Distance() = default;

    explicit Distance(double meters)
        : meters_(meters)
    {
        if (!std::isfinite(meters_) || meters_ < 0.0)
        {
            throw std::invalid_argument(
                "Distance must be finite and non-negative."
            );
        }
    }

    double meters() const noexcept
    {
        return meters_;
    }

private:
    double meters_ = 0.0;
};


class Bearing
{
public:
    Bearing() = default;

    explicit Bearing(double degrees)
        : degrees_(normalize(degrees))
    {
    }

    double degrees() const noexcept
    {
        return degrees_;
    }

private:
    static double normalize(double degrees)
    {
        if (!std::isfinite(degrees))
        {
            throw std::invalid_argument(
                "Bearing must be finite."
            );
        }

        double normalized = std::fmod(degrees, 360.0);

        if (normalized < 0.0)
        {
            normalized += 360.0;
        }

        return normalized;
    }

    double degrees_ = 0.0;
};

} // namespace navalsim::geography

