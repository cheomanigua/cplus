#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "simulation/scheduler/simulation_scheduler.hpp"

#include <cmath>
#include <limits>
#include <stdexcept>

// --------------------------------------------------
// Initial state
// --------------------------------------------------

TEST_CASE("Scheduler starts with a one second timestep")
{
    SimulationScheduler scheduler;

    CHECK(scheduler.fixedStepSeconds() == 1.0);
}

TEST_CASE("Scheduler starts with no accumulated time")
{
    SimulationScheduler scheduler;

    CHECK(scheduler.accumulatedTime() == 0.0);
}

// --------------------------------------------------
// Fixed timestep execution
// --------------------------------------------------

TEST_CASE("Scheduler does not execute an incomplete step")
{
    SimulationScheduler scheduler;

    int executedSteps = 0;

    const std::size_t count = scheduler.advance(
        0.5,
        [&executedSteps](double)
        {
            ++executedSteps;
        }
    );

    CHECK(count == 0);
    CHECK(executedSteps == 0);
    CHECK(scheduler.accumulatedTime() == 0.5);
}

TEST_CASE("Scheduler executes one complete step")
{
    SimulationScheduler scheduler;

    int executedSteps = 0;

    const std::size_t count = scheduler.advance(
        1.0,
        [&executedSteps](double)
        {
            ++executedSteps;
        }
    );

    CHECK(count == 1);
    CHECK(executedSteps == 1);
    CHECK(scheduler.accumulatedTime() == 0.0);
}

TEST_CASE("Scheduler executes multiple complete steps")
{
    SimulationScheduler scheduler;

    int executedSteps = 0;

    const std::size_t count = scheduler.advance(
        5.0,
        [&executedSteps](double)
        {
            ++executedSteps;
        }
    );

    CHECK(count == 5);
    CHECK(executedSteps == 5);
    CHECK(scheduler.accumulatedTime() == 0.0);
}

TEST_CASE("Scheduler retains fractional accumulated time")
{
    SimulationScheduler scheduler;

    int executedSteps = 0;

    scheduler.advance(0.4, [&executedSteps](double)
    {
        ++executedSteps;
    });

    scheduler.advance(0.4, [&executedSteps](double)
    {
        ++executedSteps;
    });

    scheduler.advance(0.4, [&executedSteps](double)
    {
        ++executedSteps;
    });

    CHECK(executedSteps == 1);
    CHECK(scheduler.accumulatedTime() == doctest::Approx(0.2));
}

TEST_CASE("Scheduler executes steps across multiple advances")
{
    SimulationScheduler scheduler;

    int executedSteps = 0;

    scheduler.advance(2.5, [&executedSteps](double)
    {
        ++executedSteps;
    });

    scheduler.advance(1.5, [&executedSteps](double)
    {
        ++executedSteps;
    });

    CHECK(executedSteps == 4);
    CHECK(scheduler.accumulatedTime() == 0.0);
}

// --------------------------------------------------
// Configurable timestep
// --------------------------------------------------

TEST_CASE("Scheduler supports a custom timestep")
{
    SimulationScheduler scheduler(0.5);

    int executedSteps = 0;

    const std::size_t count = scheduler.advance(
        1.5,
        [&executedSteps](double)
        {
            ++executedSteps;
        }
    );

    CHECK(scheduler.fixedStepSeconds() == 0.5);
    CHECK(count == 3);
    CHECK(executedSteps == 3);
}

TEST_CASE("Scheduler passes the fixed timestep to its callback")
{
    SimulationScheduler scheduler(0.25);

    double receivedStep = 0.0;

    scheduler.advance(0.25, [&receivedStep](double step)
    {
        receivedStep = step;
    });

    CHECK(receivedStep == 0.25);
}

// --------------------------------------------------
// Constructor validation
// --------------------------------------------------

TEST_CASE("Scheduler rejects zero timestep")
{
    CHECK_THROWS_AS(
        SimulationScheduler(0.0),
        std::invalid_argument
    );
}

TEST_CASE("Scheduler rejects negative timestep")
{
    CHECK_THROWS_AS(
        SimulationScheduler(-1.0),
        std::invalid_argument
    );
}

TEST_CASE("Scheduler rejects NaN timestep")
{
    CHECK_THROWS_AS(
        SimulationScheduler(
            std::numeric_limits<double>::quiet_NaN()
        ),
        std::invalid_argument
    );
}

TEST_CASE("Scheduler rejects infinite timestep")
{
    CHECK_THROWS_AS(
        SimulationScheduler(
            std::numeric_limits<double>::infinity()
        ),
        std::invalid_argument
    );
}

// --------------------------------------------------
// Input validation
// --------------------------------------------------

TEST_CASE("Scheduler rejects negative elapsed time")
{
    SimulationScheduler scheduler;

    CHECK_THROWS_AS(
        scheduler.advance(-1.0, [](double) {}),
        std::invalid_argument
    );
}

TEST_CASE("Scheduler rejects NaN elapsed time")
{
    SimulationScheduler scheduler;

    CHECK_THROWS_AS(
        scheduler.advance(
            std::numeric_limits<double>::quiet_NaN(),
            [](double) {}
        ),
        std::invalid_argument
    );
}

TEST_CASE("Scheduler rejects infinite elapsed time")
{
    SimulationScheduler scheduler;

    CHECK_THROWS_AS(
        scheduler.advance(
            std::numeric_limits<double>::infinity(),
            [](double) {}
        ),
        std::invalid_argument
    );
}

TEST_CASE("Scheduler rejects an empty callback")
{
    SimulationScheduler scheduler;

    CHECK_THROWS_AS(
        scheduler.advance(1.0, {}),
        std::invalid_argument
    );
}

// --------------------------------------------------
// Overflow and failure behavior
// --------------------------------------------------

TEST_CASE("Scheduler detects accumulated time overflow")
{
    const double maximum =
        std::numeric_limits<double>::max();

    SimulationScheduler scheduler(maximum);

    scheduler.advance(maximum / 2.0, [](double) {});

    CHECK_THROWS_AS(
        scheduler.advance(maximum, [](double) {}),
        std::overflow_error
    );

    CHECK(scheduler.accumulatedTime() == maximum / 2.0);
}

TEST_CASE("Scheduler retains a step when callback throws")
{
    SimulationScheduler scheduler;

    CHECK_THROWS_AS(
        scheduler.advance(
            1.5,
            [](double)
            {
                throw std::runtime_error("Test failure");
            }
        ),
        std::runtime_error
    );

    CHECK(scheduler.accumulatedTime() == 1.5);
}

