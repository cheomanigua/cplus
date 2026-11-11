
#include "data/definitions/vessel_definition.hpp"

#include <cmath>
#include <stdexcept>

namespace navalsim::data {

VesselDimensions::VesselDimensions(geography::Distance length, geography::Distance beam, geography::Distance draft)
    : length_(std::move(length)), beam_(std::move(beam)), draft_(std::move(draft)) {
    if (length_.meters() <= 0.0) {
        throw std::invalid_argument("Vessel length must be greater than zero.");
    }

    if (beam_.meters() <= 0.0) {
        throw std::invalid_argument("Vessel beam must be greater than zero.");
    }

    if (draft_.meters() <= 0.0) {
        throw std::invalid_argument("Vessel draft must be greater than zero.");
    }
}

VesselPropulsion::VesselPropulsion(double maximumSpeedMetersPerSecond)
    : maximumSpeedMetersPerSecond_(maximumSpeedMetersPerSecond) {
    if (!std::isfinite(maximumSpeedMetersPerSecond_) || maximumSpeedMetersPerSecond_ <= 0.0) {
        throw std::invalid_argument("Vessel maximum speed must be finite and greater than zero.");
    }
}

VesselDefinition::VesselDefinition(DefinitionId id, std::string name, VesselDimensions dimensions, VesselPropulsion propulsion)
    : id_(std::move(id)), name_(std::move(name)), dimensions_(std::move(dimensions)), propulsion_(std::move(propulsion)) {
    if (name_.empty()) {
        throw std::invalid_argument("Vessel definition name cannot be empty.");
    }
}

}

