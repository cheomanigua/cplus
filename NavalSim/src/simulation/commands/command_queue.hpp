#pragma once

#include <cstddef>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

#include "simulation/commands/command.hpp"

namespace navalsim::simulation
{

template <typename Command>
class CommandQueue
{
public:
    CommandId submit(Command command)
    {
        // ID zero indicates that the ID space is exhausted.
        if (nextCommandId_ == 0)
        {
            throw std::overflow_error("Command ID space is exhausted.");
        }

        const CommandId commandId = nextCommandId_;

        // Insert before advancing the counter. If allocation fails,
        // this ID remains available for the next submission.
        pendingCommands_.push_back({
            commandId,
            std::move(command)
        });

        // Preserve unique IDs for the lifetime of this queue.
        nextCommandId_ =
            commandId == std::numeric_limits<CommandId>::max()
                ? 0
                : commandId + 1;

        return commandId;
    }

    std::vector<QueuedCommand<Command>> takePending()
    {
        auto commands = std::move(pendingCommands_);
        pendingCommands_.clear();
        return commands;
    }

	const std::vector<QueuedCommand<Command>>& pendingCommands() const noexcept
	{
		return pendingCommands_;
	}

    void clear() noexcept
    {
        pendingCommands_.clear();
    }

    bool empty() const noexcept
    {
        return pendingCommands_.empty();
    }

    std::size_t size() const noexcept
    {
        return pendingCommands_.size();
    }

private:
    std::vector<QueuedCommand<Command>> pendingCommands_;
    CommandId nextCommandId_ = 1;
};

} // namespace navalsim::simulation

