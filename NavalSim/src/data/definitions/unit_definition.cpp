#include "data/definitions/unit_definition.hpp"

#include <cmath>
#include <stdexcept>
#include <string>
#include <utility>

namespace navalsim::data {
namespace {

void requirePositiveFinite(double value, const char* fieldName) {
    if (!std::isfinite(value) || value <= 0.0) {
        throw std::invalid_argument(
            std::string(fieldName) + " must be finite and greater than zero."
        );
    }
}

void requireNonNegativeFinite(double value, const char* fieldName) {
    if (!std::isfinite(value) || value < 0.0) {
        throw std::invalid_argument(
            std::string(fieldName) + " must be finite and non-negative."
        );
    }
}

} // namespace

const char* toString(UnitEnvironment environment) noexcept {
    switch (environment) {
    case UnitEnvironment::Water: return "water";
    case UnitEnvironment::Air: return "air";
    case UnitEnvironment::Land: return "land";
    }
    return "unknown";
}

UnitComponentReference::UnitComponentReference(DefinitionId componentId, std::size_t quantity)
    : componentId_(std::move(componentId)), quantity_(quantity) {
    if (quantity_ == 0) {
        throw std::invalid_argument("Component reference quantity must be greater than zero.");
    }
}

UnitDefinition::UnitDefinition(
    DefinitionId id,
    std::string name,
    std::string type,
    UnitEnvironment environment,
    double maxSpeedKnots,
    std::optional<FuelProperties> fuel,
    std::vector<UnitComponentReference> components
)
    : id_(std::move(id)),
      name_(std::move(name)),
      type_(std::move(type)),
      environment_(environment),
      maxSpeedKnots_(maxSpeedKnots),
      fuel_(std::move(fuel)),
      components_(std::move(components)) {
    if (name_.empty()) {
        throw std::invalid_argument("Unit definition name cannot be empty.");
    }
    if (type_.empty()) {
        throw std::invalid_argument("Unit definition type cannot be empty.");
    }
    if (environment_ != UnitEnvironment::Water &&
        environment_ != UnitEnvironment::Air &&
        environment_ != UnitEnvironment::Land) {
        throw std::invalid_argument("Unit environment is invalid.");
    }

    requireNonNegativeFinite(maxSpeedKnots_, "Unit maximum speed");

    if (fuel_) {
        requirePositiveFinite(fuel_->capacity, "Unit fuel capacity");
        requirePositiveFinite(fuel_->referenceSpeedKnots, "Unit fuel reference speed");
        requirePositiveFinite(fuel_->consumptionPerHour, "Unit fuel consumption per hour");
    }
}

} // namespace navalsim::data
