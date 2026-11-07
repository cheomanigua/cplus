
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "simulation/clock/simulation_clock.hpp"

#include <cmath>
#include <limits>
#include <stdexcept>

// --------------------------------------------------
// Initial state
// --------------------------------------------------

TEST_CASE("Simulation clock starts at zero")
{
    SimulationClock clock;

    CHECK(clock.simulationTime() == 0.0);
}

TEST_CASE("Simulation clock starts at normal speed")
{
    SimulationClock clock;

    CHECK(clock.timeScale() == 1.0);
}

// --------------------------------------------------
// Normal advancement
// --------------------------------------------------

TEST_CASE("Simulation clock advances at normal speed")
{
    SimulationClock clock;

    clock.advance(10.0);

    CHECK(clock.simulationTime() == 10.0);
}

TEST_CASE("Simulation clock advances at accelerated speed")
{
    SimulationClock clock;

    clock.setTimeScale(5.0);
    clock.advance(10.0);

    CHECK(clock.simulationTime() == 50.0);
}

TEST_CASE("Simulation clock allows thirty times speed")
{
    SimulationClock clock;

    clock.setTimeScale(30.0);
    clock.advance(10.0);

    CHECK(clock.simulationTime() == 300.0);
}

// --------------------------------------------------
// Time scale validation
// --------------------------------------------------

TEST_CASE("Simulation clock rejects zero time scale")
{
    SimulationClock clock;

    CHECK_THROWS_AS(
        clock.setTimeScale(0.0),
        std::invalid_argument
    );
}

TEST_CASE("Simulation clock rejects negative time scale")
{
    SimulationClock clock;

    CHECK_THROWS_AS(
        clock.setTimeScale(-1.0),
        std::invalid_argument
    );
}

TEST_CASE("Simulation clock rejects NaN time scale")
{
    SimulationClock clock;

    CHECK_THROWS_AS(
        clock.setTimeScale(
            std::numeric_limits<double>::quiet_NaN()
        ),
        std::invalid_argument
    );
}

TEST_CASE("Simulation clock rejects infinite time scale")
{
    SimulationClock clock;

    CHECK_THROWS_AS(
        clock.setTimeScale(
            std::numeric_limits<double>::infinity()
        ),
        std::invalid_argument
    );
}

TEST_CASE("Invalid time scale does not change existing time scale")
{
    SimulationClock clock;

    clock.setTimeScale(5.0);

    CHECK_THROWS_AS(
        clock.setTimeScale(0.0),
        std::invalid_argument
    );

    CHECK(clock.timeScale() == 5.0);
}

// --------------------------------------------------
// Delta time validation
// --------------------------------------------------

TEST_CASE("Simulation clock rejects negative delta time")
{
    SimulationClock clock;

    CHECK_THROWS_AS(
        clock.advance(-1.0),
        std::invalid_argument
    );
}

TEST_CASE("Simulation clock rejects NaN delta time")
{
    SimulationClock clock;

    CHECK_THROWS_AS(
        clock.advance(
            std::numeric_limits<double>::quiet_NaN()
        ),
        std::invalid_argument
    );
}

TEST_CASE("Simulation clock rejects infinite delta time")
{
    SimulationClock clock;

    CHECK_THROWS_AS(
        clock.advance(
            std::numeric_limits<double>::infinity()
        ),
        std::invalid_argument
    );
}

// --------------------------------------------------
// Arithmetic overflow
// --------------------------------------------------

TEST_CASE("Simulation clock detects multiplication overflow")
{
    SimulationClock clock;

    clock.setTimeScale(
        std::numeric_limits<double>::max()
    );

    CHECK_THROWS_AS(
        clock.advance(2.0),
        std::overflow_error
    );

    CHECK(clock.simulationTime() == 0.0);
}

TEST_CASE("Simulation clock detects accumulated time overflow")
{
    SimulationClock clock;

    const double maximum =
        std::numeric_limits<double>::max();

    clock.advance(maximum);

    CHECK(clock.simulationTime() == maximum);

    CHECK_THROWS_AS(
        clock.advance(maximum),
        std::overflow_error
    );

    CHECK(clock.simulationTime() == maximum);
}

