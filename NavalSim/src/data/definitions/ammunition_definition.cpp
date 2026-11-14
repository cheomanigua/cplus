#include "data/definitions/ammunition_definition.hpp"

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

AmmunitionDefinition::AmmunitionDefinition(
    DefinitionId id,
    std::string name,
    double range,
    double speed,
    bool guided,
    double damage
)
    : id_(std::move(id)),
      name_(std::move(name)),
      range_(range),
      speed_(speed),
      guided_(guided),
      damage_(damage) {
    if (name_.empty()) {
        throw std::invalid_argument("Ammunition definition name cannot be empty.");
    }
    requirePositiveFinite(range_, "Ammunition range");
    requirePositiveFinite(speed_, "Ammunition speed");
    requirePositiveFinite(damage_, "Ammunition damage");
}

} // namespace navalsim::data
