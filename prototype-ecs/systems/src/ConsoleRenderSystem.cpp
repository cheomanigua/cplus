#include "ConsoleRenderSystem.h"
#include "PositionComponent.h"
#include <iostream>
#include <raylib.h>

void ConsoleRenderSystem::update(Registry& registry)
{
    static double lastLogTime{0.0};
    double currentTime = GetTime();

    if (currentTime - lastLogTime >= 1.0)
    {
        std::cout << "[ConsoleRenderSystem] Rendering entities to console:\n";
        
        for (Entity e : registry.getEntities())
        {
            bool hasPos = registry.hasComponent<PositionComponent>(e);
            std::cout << "  > Checking Entity " << e << " -> hasComponent: " << (hasPos ? "true" : "false") << "\n";

            if (hasPos)
            {
                auto& pos = registry.getComponent<PositionComponent>(e);
                std::cout << "    Position -> X: " << pos.position.x << ", Y: " << pos.position.y << "\n";
            }
        }
        
        lastLogTime = currentTime;
    }
}
