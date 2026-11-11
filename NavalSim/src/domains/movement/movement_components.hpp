
#pragma once

#include <cmath>
#include <stdexcept>

#include "core/geography/geographic_types.hpp"

namespace navalsim::movement
{

class Heading
{
public:
    Heading() = default;

    explicit Heading(double degrees)
        : bearing_(degrees)
    {
    }

    double degrees() const noexcept
    {
        return bearing_.degrees();
    }

    void setDegrees(double degrees)
    {
        bearing_ = geography::Bearing(degrees);
    }

private:
    geography::Bearing bearing_;
};



class DesiredHeading
{
public:
    DesiredHeading() = default;

    explicit DesiredHeading(double degrees)
        : bearing_(degrees)
    {
    }

    double degrees() const noexcept
    {
        return bearing_.degrees();
    }

    void setDegrees(double degrees)
    {
        bearing_ = geography::Bearing(degrees);
    }

private:
    geography::Bearing bearing_;
};



class Speed
{
public:
    Speed() = default;

    explicit Speed(double metersPerSecond)
        : metersPerSecond_(metersPerSecond)
    {
        if (!std::isfinite(metersPerSecond_) ||
            metersPerSecond_ < 0.0)
        {
            throw std::invalid_argument(
                "Speed must be finite and non-negative."
            );
        }
    }

    double metersPerSecond() const noexcept
    {
        return metersPerSecond_;
    }

private:
    double metersPerSecond_ = 0.0;
};



class MaximumSpeed {
public:
    MaximumSpeed() = default;
    explicit MaximumSpeed(double metersPerSecond) : metersPerSecond_(metersPerSecond) {
        if (!std::isfinite(metersPerSecond_) || metersPerSecond_ < 0.0)
            throw std::invalid_argument("Maximum speed must be finite and non-negative.");
    }
    double metersPerSecond() const noexcept { return metersPerSecond_; }
private:
    double metersPerSecond_ = 0.0;
};



class TurnRate
{
public:
    TurnRate() = default;

    explicit TurnRate(double degreesPerSecond)
        : degreesPerSecond_(degreesPerSecond)
    {
        if (!std::isfinite(degreesPerSecond_) ||
            degreesPerSecond_ < 0.0)
        {
            throw std::invalid_argument(
                "Turn rate must be finite and non-negative."
            );
        }
    }

    double degreesPerSecond() const noexcept
    {
        return degreesPerSecond_;
    }

private:
    double degreesPerSecond_ = 0.0;
};



enum class TurnDirection
{
    None,
    Port,
    Starboard
};

enum class MovementStatus
{
    Stopped,
    Underway
};

} // namespace navalsim::movement

