#pragma once

#include <optional>
#include <string>
#include <variant>

#include "data/definitions/definition_id.hpp"

namespace navalsim::data {

enum class ComponentCategory {
    DamageOnly,
    Simulation
};

const char* toString(ComponentCategory category) noexcept;

class RadarProperties {
public:
    explicit RadarProperties(double rangeNm);
    double rangeNm() const noexcept { return rangeNm_; }

private:
    double rangeNm_;
};

using ComponentProperties = std::variant<RadarProperties>;

class ComponentDefinition {
public:
    ComponentDefinition(
        DefinitionId id,
        std::string name,
        ComponentCategory category,
        std::string type,
        std::optional<ComponentProperties> properties = std::nullopt
    );

    const DefinitionId& id() const noexcept { return id_; }
    const std::string& name() const noexcept { return name_; }
    ComponentCategory category() const noexcept { return category_; }
    const std::string& type() const noexcept { return type_; }
    const std::optional<ComponentProperties>& properties() const noexcept { return properties_; }

private:
    DefinitionId id_;
    std::string name_;
    ComponentCategory category_;
    std::string type_;
    std::optional<ComponentProperties> properties_;
};

} // namespace navalsim::data
