#include "simulation/scheduler/simulation_scheduler.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

SimulationScheduler::SimulationScheduler(double fixedStepSeconds)
    : fixedStepSeconds_(fixedStepSeconds)
{
    if (!std::isfinite(fixedStepSeconds_) ||
        fixedStepSeconds_ <= 0.0)
    {
        throw std::invalid_argument(
            "Fixed timestep must be finite and greater than zero."
        );
    }
}

void SimulationScheduler::validateAdvance(
    double elapsedSimulationSeconds) const
{
    if (!std::isfinite(elapsedSimulationSeconds) ||
        elapsedSimulationSeconds < 0.0)
    {
        throw std::invalid_argument(
            "Elapsed simulation time must be finite and non-negative."
        );
    }

    const double newAccumulatedTime =
        accumulatedTime_ + elapsedSimulationSeconds;

    if (!std::isfinite(newAccumulatedTime))
    {
        throw std::overflow_error(
            "Scheduler accumulated time overflowed."
        );
    }
}

std::size_t SimulationScheduler::advance(
    double elapsedSimulationSeconds,
    const StepCallback& stepCallback)
{
    validateAdvance(elapsedSimulationSeconds);

    if (!stepCallback)
    {
        throw std::invalid_argument(
            "Simulation step callback cannot be empty."
        );
    }

    accumulatedTime_ += elapsedSimulationSeconds;

    std::size_t executedSteps = 0;

    while (true)
    {
        // Allow for tiny floating-point rounding differences.
        const double tolerance =
            8.0 *
            std::numeric_limits<double>::epsilon() *
            std::max({
                1.0,
                std::abs(accumulatedTime_),
                fixedStepSeconds_
            });

        if (accumulatedTime_ + tolerance < fixedStepSeconds_)
        {
            break;
        }

        // Do not consume the step until its callback succeeds.
        stepCallback(fixedStepSeconds_);

        accumulatedTime_ -= fixedStepSeconds_;

        // Avoid retaining a tiny negative rounding error.
        if (accumulatedTime_ < 0.0 &&
            accumulatedTime_ >= -tolerance)
        {
            accumulatedTime_ = 0.0;
        }

        ++executedSteps;
    }

    return executedSteps;
}

void SimulationScheduler::reset() noexcept
{
    accumulatedTime_ = 0.0;
}

double SimulationScheduler::fixedStepSeconds() const noexcept
{
    return fixedStepSeconds_;
}

double SimulationScheduler::accumulatedTime() const noexcept
{
    return accumulatedTime_;
}
