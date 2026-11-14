#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "data/definitions/definition_id.hpp"

namespace navalsim::data {

enum class UnitEnvironment {
    Water,
    Air,
    Land
};

const char* toString(UnitEnvironment environment) noexcept;

struct FuelProperties {
    double capacity;
    double referenceSpeedKnots;
    double consumptionPerHour;
};

class UnitComponentReference {
public:
    UnitComponentReference(DefinitionId componentId, std::size_t quantity);

    const DefinitionId& componentId() const noexcept { return componentId_; }
    std::size_t quantity() const noexcept { return quantity_; }

private:
    DefinitionId componentId_;
    std::size_t quantity_;
};

class UnitDefinition {
public:
    UnitDefinition(
        DefinitionId id,
        std::string name,
        std::string type,
        UnitEnvironment environment,
        double maxSpeedKnots,
        std::optional<FuelProperties> fuel,
        std::vector<UnitComponentReference> components
    );

    const DefinitionId& id() const noexcept { return id_; }
    const std::string& name() const noexcept { return name_; }
    const std::string& type() const noexcept { return type_; }
    UnitEnvironment environment() const noexcept { return environment_; }
    double maxSpeedKnots() const noexcept { return maxSpeedKnots_; }
    const std::optional<FuelProperties>& fuel() const noexcept { return fuel_; }
    const std::vector<UnitComponentReference>& components() const noexcept { return components_; }

private:
    DefinitionId id_;
    std::string name_;
    std::string type_;
    UnitEnvironment environment_;
    double maxSpeedKnots_;
    std::optional<FuelProperties> fuel_;
    std::vector<UnitComponentReference> components_;
};

} // namespace navalsim::data
