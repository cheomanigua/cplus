#include "simulation/clock/simulation_clock.hpp"

#include <cmath>
#include <stdexcept>

double SimulationClock::previewAdvance(
    double realDeltaSeconds) const
{
    if (!std::isfinite(realDeltaSeconds) ||
        realDeltaSeconds < 0.0)
    {
        throw std::invalid_argument(
            "Real delta time must be finite and non-negative."
        );
    }

    const double scaledDelta =
        realDeltaSeconds * timeScale_;

    if (!std::isfinite(scaledDelta))
    {
        throw std::overflow_error(
            "Simulation time increment overflowed."
        );
    }

    const double newSimulationTime =
        simulationTime_ + scaledDelta;

    if (!std::isfinite(newSimulationTime))
    {
        throw std::overflow_error(
            "Simulation time overflowed."
        );
    }

    return scaledDelta;
}

void SimulationClock::advance(double realDeltaSeconds)
{
    const double scaledDelta =
        previewAdvance(realDeltaSeconds);

    simulationTime_ += scaledDelta;
}

void SimulationClock::setTimeScale(double timeScale)
{
    if (!std::isfinite(timeScale) || timeScale <= 0.0)
    {
        throw std::invalid_argument(
            "Time scale must be finite and greater than zero."
        );
    }

    timeScale_ = timeScale;
}

void SimulationClock::reset() noexcept
{
    simulationTime_ = 0.0;
    timeScale_ = 1.0;
}

double SimulationClock::simulationTime() const noexcept
{
    return simulationTime_;
}

double SimulationClock::timeScale() const noexcept
{
    return timeScale_;
}
