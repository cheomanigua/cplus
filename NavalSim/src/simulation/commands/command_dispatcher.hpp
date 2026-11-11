#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <stdexcept>
#include <typeindex>
#include <unordered_map>
#include <utility>

#include "simulation/commands/command.hpp"

namespace navalsim::simulation
{

class CommandDispatcher
{
private:
    class IHandler
    {
    public:
        virtual ~IHandler() = default;
    };

    template <typename Context, typename Command, typename Result>
    class Handler final : public IHandler
    {
    public:
        using Function = std::function<Result(Context&, const Command&)>;

        explicit Handler(Function function)
            : function_(std::move(function))
        {
        }

        Result execute(Context& context, const Command& command) const
        {
            return function_(context, command);
        }

    private:
        Function function_;
    };

public:
    template <typename Context, typename Command, typename Result, typename Callable>
    void registerHandler(Callable&& callable)
    {
        const std::type_index commandType(typeid(Command));

        if (handlers_.contains(commandType))
        {
            throw std::logic_error("A handler is already registered for this command type.");
        }

        using Function = typename Handler<Context, Command, Result>::Function;

        Function function(std::forward<Callable>(callable));

        if (!function)
        {
            throw std::invalid_argument("Cannot register an empty command handler.");
        }

        handlers_.emplace(
            commandType,
            std::make_unique<Handler<Context, Command, Result>>(std::move(function))
        );
    }

    template <typename Context, typename Command, typename Result>
    CommandResult<Result> dispatch(
        Context& context,
        CommandId commandId,
        const Command& command
    ) const
    {
        const std::type_index commandType(typeid(Command));
        const auto iterator = handlers_.find(commandType);

        if (iterator == handlers_.end())
        {
            throw std::logic_error("No handler is registered for this command type.");
        }

        auto* handler = dynamic_cast<Handler<Context, Command, Result>*>(
            iterator->second.get()
        );

        if (handler == nullptr)
        {
            throw std::logic_error(
                "The registered command handler has incompatible context or result types."
            );
        }

        return {
            commandId,
            handler->execute(context, command)
        };
    }

    template <typename Command>
    bool hasHandler() const
    {
        return handlers_.contains(std::type_index(typeid(Command)));
    }

    void clear() noexcept
    {
        handlers_.clear();
    }

    std::size_t size() const noexcept
    {
        return handlers_.size();
    }

private:
    std::unordered_map<std::type_index, std::unique_ptr<IHandler>> handlers_;
};

} // namespace navalsim::simulation

