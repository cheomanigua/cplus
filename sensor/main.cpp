#include <iostream>

enum class SensorState {
    Disabled,
    Passive,
    Active
};

struct PositionComp {
    float x{};
    float y{};
};

struct SensorComp {
    float range{};
    SensorState state{SensorState::Disabled};
};

bool IsWithinRadarRange(
        const PositionComp& sourcePos,
        const PositionComp& targetPos,
        const SensorComp& radar)
{
    const float deltaX { targetPos.x - sourcePos.x };
    const float deltaY { targetPos.y - sourcePos.y };
    const float distanceSquared { (deltaX * deltaX) + (deltaY * deltaY) };
    float rangeSquared {radar.range * radar.range};

    switch (radar.state)
    {
        case SensorState::Disabled:
            std::cout << "Sensor disabled\n";
            rangeSquared = 0.0f;
            break;
        case SensorState::Passive:
            std::cout << "Passive scanning\n";
            rangeSquared /= 2.0f;
            break;
        case SensorState::Active:
            std::cout << "Active scanning\n";
            break;
    }

    std::cout << "Distance: " << distanceSquared 
              << ". Radar Range: " << rangeSquared << "\n";

    return distanceSquared <= rangeSquared;
}

int main() {
    // Stack allocation (standard behavior for structs in C++)
    PositionComp ussPasadenaPos {110.00f, 30.00f};
    PositionComp opforShipPos {110.00f, 80.01f};
    SensorComp ussPasadenaRadar {50.0f, SensorState::Active};

    bool detected { IsWithinRadarRange(ussPasadenaPos, opforShipPos, ussPasadenaRadar) };

    std::cout << "Target Detected: " << (detected ? "True" : "False") << "\n";

    return 0;
}
