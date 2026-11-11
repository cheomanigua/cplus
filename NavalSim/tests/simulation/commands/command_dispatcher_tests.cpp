#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <stdexcept>

#include "simulation/commands/command_dispatcher.hpp"

namespace {

struct TestContext {
    int offset = 0;
};

struct FirstCommand {
    int value = 0;
};

struct SecondCommand {
    int value = 0;
};

enum class TestStatus {
    Applied,
    Rejected
};

} // namespace

TEST_CASE("Command dispatcher registers and invokes a typed handler") {
    navalsim::simulation::CommandDispatcher dispatcher;
    TestContext context{5};

    dispatcher.registerHandler<TestContext, FirstCommand, TestStatus>(
        [](TestContext& handlerContext, const FirstCommand& command) {
            handlerContext.offset += command.value;
            return TestStatus::Applied;
        }
    );

    const auto result = dispatcher.dispatch<TestContext, FirstCommand, TestStatus>(
        context, 42, FirstCommand{10}
    );

    CHECK(result.commandId == 42);
    CHECK(result.status == TestStatus::Applied);
    CHECK(context.offset == 15);
}

TEST_CASE("Command dispatcher routes different command types to their handlers") {
    navalsim::simulation::CommandDispatcher dispatcher;
    TestContext context{};

    dispatcher.registerHandler<TestContext, FirstCommand, TestStatus>(
        [](TestContext& handlerContext, const FirstCommand& command) {
            handlerContext.offset += command.value;
            return TestStatus::Applied;
        }
    );

    dispatcher.registerHandler<TestContext, SecondCommand, TestStatus>(
        [](TestContext& handlerContext, const SecondCommand& command) {
            handlerContext.offset -= command.value;
            return TestStatus::Rejected;
        }
    );

    const auto firstResult = dispatcher.dispatch<TestContext, FirstCommand, TestStatus>(
        context, 1, FirstCommand{10}
    );

    const auto secondResult = dispatcher.dispatch<TestContext, SecondCommand, TestStatus>(
        context, 2, SecondCommand{3}
    );

    CHECK(firstResult.commandId == 1);
    CHECK(firstResult.status == TestStatus::Applied);
    CHECK(secondResult.commandId == 2);
    CHECK(secondResult.status == TestStatus::Rejected);
    CHECK(context.offset == 7);
}

TEST_CASE("Command dispatcher reports registered command types") {
    navalsim::simulation::CommandDispatcher dispatcher;

    CHECK_FALSE(dispatcher.hasHandler<FirstCommand>());

    dispatcher.registerHandler<TestContext, FirstCommand, TestStatus>(
        [](TestContext&, const FirstCommand&) {
            return TestStatus::Applied;
        }
    );

    CHECK(dispatcher.hasHandler<FirstCommand>());
    CHECK_FALSE(dispatcher.hasHandler<SecondCommand>());
    CHECK(dispatcher.size() == 1);
}

TEST_CASE("Command dispatcher rejects duplicate handler registration") {
    navalsim::simulation::CommandDispatcher dispatcher;

    const auto handler = [](TestContext&, const FirstCommand&) {
        return TestStatus::Applied;
    };

    dispatcher.registerHandler<TestContext, FirstCommand, TestStatus>(handler);

    CHECK_THROWS_AS(
        (dispatcher.registerHandler<TestContext, FirstCommand, TestStatus>(handler)),
        std::logic_error
    );

    CHECK(dispatcher.size() == 1);
}

TEST_CASE("Command dispatcher rejects dispatch when no handler is registered") {
    navalsim::simulation::CommandDispatcher dispatcher;
    TestContext context{};

    CHECK_THROWS_AS(
        (dispatcher.dispatch<TestContext, FirstCommand, TestStatus>(
            context, 1, FirstCommand{10}
        )),
        std::logic_error
    );
}

TEST_CASE("Command dispatcher rejects incompatible result types") {
    navalsim::simulation::CommandDispatcher dispatcher;
    TestContext context{};

    dispatcher.registerHandler<TestContext, FirstCommand, TestStatus>(
        [](TestContext&, const FirstCommand&) {
            return TestStatus::Applied;
        }
    );

    CHECK_THROWS_AS(
        (dispatcher.dispatch<TestContext, FirstCommand, int>(
            context, 1, FirstCommand{10}
        )),
        std::logic_error
    );
}

TEST_CASE("Command dispatcher clear removes all registered handlers") {
    navalsim::simulation::CommandDispatcher dispatcher;

    dispatcher.registerHandler<TestContext, FirstCommand, TestStatus>(
        [](TestContext&, const FirstCommand&) {
            return TestStatus::Applied;
        }
    );

    dispatcher.registerHandler<TestContext, SecondCommand, TestStatus>(
        [](TestContext&, const SecondCommand&) {
            return TestStatus::Rejected;
        }
    );

    CHECK(dispatcher.size() == 2);

    dispatcher.clear();

    CHECK(dispatcher.size() == 0);
    CHECK_FALSE(dispatcher.hasHandler<FirstCommand>());
    CHECK_FALSE(dispatcher.hasHandler<SecondCommand>());
}
