#include <iostream>

struct PositionComp {
    float x{}, y{};
};

struct SensorComp {
    float range{};
    bool enabled{};
};

bool IsWithinRadarRange(
        const PositionComp& sourcePos,
        const PositionComp& targetPos,
        const SensorComp& radar)
{
    float deltaX {targetPos.x - sourcePos.x};
    float deltaY {targetPos.y - sourcePos.y};
    float distanceSquared {(deltaX * deltaX) + (deltaY * deltaY)};
    float rangeSquared {radar.range * radar.range};

    if (!radar.enabled)
        return false;

    std::cout << "Distance: " << distanceSquared 
              << ". Radar Range: " << rangeSquared << "\n";

    return distanceSquared <= rangeSquared;
}

int main() {
    // Stack allocation (standard behavior for structs in C++)
    PositionComp bluePos {110.00f, 30.00f};
    PositionComp redPos {110.00f, 80.01f};
    SensorComp radar {50.0f, true};

    bool detected = IsWithinRadarRange(bluePos, redPos, radar);

    std::cout << "Target Detected: " << (detected ? "True" : "False") << "\n";

    return 0;
}
