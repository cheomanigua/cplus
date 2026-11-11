#pragma once

#include <cstdint>
#include <exception>
#include <string>

namespace navalsim::simulation
{

using CommandId = std::uint64_t;

template <typename Command>
struct QueuedCommand
{
    CommandId id;
    Command command;
};

template <typename Status>
struct CommandResult
{
    CommandId commandId;
    Status status;
};

struct CommandExecutionFailure
{
    CommandId commandId;
    std::string message;
    std::exception_ptr exception;
};

} // namespace navalsim::simulation

