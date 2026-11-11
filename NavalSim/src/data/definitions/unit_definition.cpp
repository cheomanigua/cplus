#include "data/definitions/unit_definition.hpp"

#include <cmath>
#include <stdexcept>
#include <string>

namespace navalsim::data {
namespace {

void requirePositiveFinite(double value, const char* fieldName) {
    if (!std::isfinite(value) || value <= 0.0) {
        throw std::invalid_argument(std::string(fieldName) + " must be finite and greater than zero.");
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

UnitPhysicalProperties::UnitPhysicalProperties(double massKg, double lengthMeters, double widthMeters)
    : massKg_(massKg), lengthMeters_(lengthMeters), widthMeters_(widthMeters) {
    requirePositiveFinite(massKg_, "Unit mass");
    requirePositiveFinite(lengthMeters_, "Unit length");
    requirePositiveFinite(widthMeters_, "Unit width");
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
    UnitPhysicalProperties physicalProperties,
    std::vector<UnitComponentReference> components
)
    : id_(std::move(id)),
      name_(std::move(name)),
      type_(std::move(type)),
      environment_(environment),
      physicalProperties_(std::move(physicalProperties)),
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
}

} // namespace navalsim::data
