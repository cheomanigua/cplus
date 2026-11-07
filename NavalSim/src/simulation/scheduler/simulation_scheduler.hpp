#pragma once

#include <cstddef>
#include <functional>

class SimulationScheduler
{
public:
    using StepCallback = std::function<void(double)>;

    explicit SimulationScheduler(double fixedStepSeconds = 1.0);
	void validateAdvance(double elapsedSimulationSeconds) const;

    // Adds elapsed simulation time and executes available fixed steps.
    // Returns the number of steps executed.
    std::size_t advance(
        double elapsedSimulationSeconds,
        const StepCallback& stepCallback
    );

	void reset() noexcept;

    double fixedStepSeconds() const noexcept;
    double accumulatedTime() const noexcept;

private:
    double fixedStepSeconds_;
    double accumulatedTime_ = 0.0;
};

