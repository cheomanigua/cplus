#include <iostream>
#include "raylib.h"
#include "raymath.h"

enum class SensorState {
    Disabled,
    Passive,
    Active
};

struct PositionComp {
    Vector2 position{};
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
    const float distanceSquared { Vector2DistanceSqr(sourcePos.position, targetPos.position) };
    float rangeSquared { radar.range * radar.range };

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
    PositionComp opforShipPos {160.00f, 30.00f};
    SensorComp ussPasadenaRadar {50.0f, SensorState::Passive};

    bool detected { IsWithinRadarRange(ussPasadenaPos, opforShipPos, ussPasadenaRadar) };

    std::cout << "Target Detected: " << (detected ? "True" : "False") << "\n";

    return 0;
}
