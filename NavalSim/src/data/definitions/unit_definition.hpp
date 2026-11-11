#pragma once

#include <cstddef>
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

class UnitPhysicalProperties {
public:
    UnitPhysicalProperties(double massKg, double lengthMeters, double widthMeters);

    double massKg() const noexcept { return massKg_; }
    double lengthMeters() const noexcept { return lengthMeters_; }
    double widthMeters() const noexcept { return widthMeters_; }

private:
    double massKg_;
    double lengthMeters_;
    double widthMeters_;
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
        UnitPhysicalProperties physicalProperties,
        std::vector<UnitComponentReference> components
    );

    const DefinitionId& id() const noexcept { return id_; }
    const std::string& name() const noexcept { return name_; }
    const std::string& type() const noexcept { return type_; }
    UnitEnvironment environment() const noexcept { return environment_; }
    const UnitPhysicalProperties& physicalProperties() const noexcept { return physicalProperties_; }
    const std::vector<UnitComponentReference>& components() const noexcept { return components_; }

private:
    DefinitionId id_;
    std::string name_;
    std::string type_;
    UnitEnvironment environment_;
    UnitPhysicalProperties physicalProperties_;
    std::vector<UnitComponentReference> components_;
};

} // namespace navalsim::data
