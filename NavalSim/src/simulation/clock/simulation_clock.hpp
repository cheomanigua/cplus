#pragma once

class SimulationClock
{
public:
    SimulationClock() = default;

    void advance(double realDeltaSeconds);
    double previewAdvance(double realDeltaSeconds) const;

    void setTimeScale(double timeScale);
    void reset() noexcept;

    double simulationTime() const noexcept;
    double timeScale() const noexcept;

private:
    double simulationTime_ = 0.0;
    double timeScale_ = 1.0;
};
