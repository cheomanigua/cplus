#pragma once

#include <string>
#include <utility>

#include "data/definitions/definition_id.hpp"

namespace navalsim::data {

class AmmunitionDefinition {
public:
    AmmunitionDefinition(
        DefinitionId id,
        std::string name,
        double range,
        double speed,
        bool guided,
        double damage
    );

    const DefinitionId& id() const noexcept { return id_; }
    const std::string& name() const noexcept { return name_; }
    double range() const noexcept { return range_; }
    double speed() const noexcept { return speed_; }
    bool isGuided() const noexcept { return guided_; }
    double damage() const noexcept { return damage_; }

private:
    DefinitionId id_;
    std::string name_;
    double range_;
    double speed_;
    bool guided_;
    double damage_;
};

} // namespace navalsim::data
