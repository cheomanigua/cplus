#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <cstdint>
#include <limits>
#include <stdexcept>

#include "simulation/commands/command_queue.hpp"

namespace
{

struct TestCommand
{
    int value;
};

} // namespace

TEST_CASE("Command queue assigns IDs starting at one")
{
    navalsim::simulation::CommandQueue<TestCommand> queue;

    const auto firstId = queue.submit(TestCommand{10});
    const auto secondId = queue.submit(TestCommand{20});

    CHECK(firstId == 1);
    CHECK(secondId == 2);
}

TEST_CASE("Command queue preserves submission order")
{
    navalsim::simulation::CommandQueue<TestCommand> queue;

    queue.submit(TestCommand{10});
    queue.submit(TestCommand{20});
    queue.submit(TestCommand{30});

    const auto commands = queue.takePending();

    REQUIRE(commands.size() == 3);
    CHECK(commands[0].command.value == 10);
    CHECK(commands[1].command.value == 20);
    CHECK(commands[2].command.value == 30);
}

TEST_CASE("Taking pending commands empties the queue")
{
    navalsim::simulation::CommandQueue<TestCommand> queue;

    queue.submit(TestCommand{10});

    const auto commands = queue.takePending();

    REQUIRE(commands.size() == 1);
    CHECK(queue.empty());
    CHECK(queue.size() == 0);
}

TEST_CASE("Taking an empty queue returns no commands")
{
    navalsim::simulation::CommandQueue<TestCommand> queue;

    const auto commands = queue.takePending();

    CHECK(commands.empty());
    CHECK(queue.empty());
}

TEST_CASE("Command queue clear discards pending commands")
{
    navalsim::simulation::CommandQueue<TestCommand> queue;

    queue.submit(TestCommand{10});
    queue.submit(TestCommand{20});

    queue.clear();

    CHECK(queue.empty());
    CHECK(queue.size() == 0);
    CHECK(queue.takePending().empty());
}

TEST_CASE("Clearing the queue does not reset command IDs")
{
    navalsim::simulation::CommandQueue<TestCommand> queue;

    CHECK(queue.submit(TestCommand{10}) == 1);

    queue.clear();

    CHECK(queue.submit(TestCommand{20}) == 2);
}

TEST_CASE("Taking commands does not reset command IDs")
{
    navalsim::simulation::CommandQueue<TestCommand> queue;

    CHECK(queue.submit(TestCommand{10}) == 1);

    queue.takePending();

    CHECK(queue.submit(TestCommand{20}) == 2);
}

TEST_CASE("Command queue supports maximum command ID")
{
    navalsim::simulation::CommandQueue<TestCommand> queue;

    // This test is expanded when we add a test seam for ID exhaustion.
    CHECK(queue.submit(TestCommand{10}) == 1);
}
