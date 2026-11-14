#include "data/definitions/component_definition.hpp"

#include <cmath>
#include <stdexcept>
#include <string>
#include <utility>

namespace navalsim::data {
namespace {

void requirePositiveFinite(double value, const char* fieldName) {
    if (!std::isfinite(value) || value <= 0.0) {
        throw std::invalid_argument(std::string(fieldName) + " must be finite and greater than zero.");
    }
}

} // namespace

const char* toString(ComponentCategory category) noexcept {
    switch (category) {
    case ComponentCategory::DamageOnly: return "damage_only";
    case ComponentCategory::Simulation: return "simulation";
    }
    return "unknown";
}

RadarProperties::RadarProperties(double rangeNm) : rangeNm_(rangeNm) {
    requirePositiveFinite(rangeNm_, "Radar range");
}

ComponentDefinition::ComponentDefinition(
    DefinitionId id,
    std::string name,
    ComponentCategory category,
    std::string type,
    std::optional<ComponentProperties> properties
)
    : id_(std::move(id)),
      name_(std::move(name)),
      category_(category),
      type_(std::move(type)),
      properties_(std::move(properties)) {
    if (name_.empty()) {
        throw std::invalid_argument("Component definition name cannot be empty.");
    }
    if (type_.empty()) {
        throw std::invalid_argument("Component definition type cannot be empty.");
    }
    if (category_ != ComponentCategory::DamageOnly &&
        category_ != ComponentCategory::Simulation) {
        throw std::invalid_argument("Component category is invalid.");
    }
    if (category_ == ComponentCategory::DamageOnly && properties_) {
        throw std::invalid_argument("Damage-only components cannot define simulation properties.");
    }
    if (type_ == "radar") {
        if (category_ != ComponentCategory::Simulation ||
            !properties_ ||
            !std::holds_alternative<RadarProperties>(*properties_)) {
            throw std::invalid_argument("Radar components require simulation category and radar properties.");
        }
    } else if (properties_) {
        throw std::invalid_argument("No simulation properties are defined for component type: " + type_);
    }
}

} // namespace navalsim::data
