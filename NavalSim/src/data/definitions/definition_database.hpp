#pragma once

#include <concepts>
#include <memory>
#include <stdexcept>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

#include "data/definitions/definition_id.hpp"

namespace navalsim::data {

template<typename T>
concept TypedDefinition = requires(const T& definition) {
    { definition.id() } -> std::same_as<const DefinitionId&>;
};

class DefinitionDatabase {
public:
    template<TypedDefinition T>
    void add(T definition) {
        const std::string id = definition.id().value();
        auto storedDefinition = std::make_shared<const T>(std::move(definition));
        auto [iterator, inserted] = definitions_.try_emplace(
            id,
            Entry{std::type_index(typeid(T)), std::move(storedDefinition)}
        );
        if (!inserted) throw std::invalid_argument("Duplicate definition ID: " + id);
    }

    template<TypedDefinition T>
    void addBatch(std::vector<T> definitions) {
        auto updatedDefinitions = definitions_;

        for (auto& definition : definitions) {
            const std::string id = definition.id().value();
            auto storedDefinition = std::make_shared<const T>(std::move(definition));
            auto [iterator, inserted] = updatedDefinitions.try_emplace(
                id,
                Entry{std::type_index(typeid(T)), std::move(storedDefinition)}
            );
            if (!inserted) throw std::invalid_argument("Duplicate definition ID: " + id);
        }

        definitions_.swap(updatedDefinitions);
    }

    bool contains(const DefinitionId& id) const {
        return definitions_.contains(id.value());
    }

    template<TypedDefinition T>
    const T* find(const DefinitionId& id) const {
        const auto iterator = definitions_.find(id.value());
        if (iterator == definitions_.end()) return nullptr;
        if (iterator->second.type != std::type_index(typeid(T))) return nullptr;
        return static_cast<const T*>(iterator->second.definition.get());
    }

    std::size_t definitionCount() const noexcept {
        return definitions_.size();
    }

private:
    struct Entry {
        std::type_index type;
        std::shared_ptr<const void> definition;
    };

    std::unordered_map<std::string, Entry> definitions_;
};

} // namespace
