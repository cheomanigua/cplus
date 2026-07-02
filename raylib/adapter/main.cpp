#include "SimMath.hpp"
#include "SimInput.hpp"
#include "SimRenderer.hpp"
#include "Colors.hpp"

namespace SimulationEngine {
    class Entity {
    private:
        SimEngine::Vec2 position;
    public:
        Entity(float x, float y) : position({x, y}) {}
        void SetPos(SimEngine::Vec2 pos) { position = pos; }
        SimEngine::Vec2 GetPos() const { return position; }
    };
}

int main() {
    // 1. Initialization
    SimEngine::Renderer::Init(800, 600, "Harpoon Sim - Engine Agnostic Core");
    SimEngine::Renderer::SetTargetFPS(60); // Prevents CPU/Fan overload

    SimulationEngine::Entity unknownTarget(220.0f, 230.0f);
    SimulationEngine::Entity ussPasadena(520.0f, 330.0f);
    SimEngine::Vec2 destination = ussPasadena.GetPos();
    bool isMoving = false;
    const float speed = 200.0f;

    // 2. Main Simulation Loop
    while (!SimEngine::Renderer::ShouldClose()) {
        float dt = SimEngine::Renderer::GetFrameTime();

        // POLL AGNOSTIC INTENT
        SimEngine::InputIntent intent = SimEngine::InputAdapter::PollInputs();

        if (intent.moveRequested) {
            destination = intent.destination;
            isMoving = true;
        }

        // AGNOSTIC MOVEMENT LOGIC
        if (isMoving) {
            SimEngine::Vec2 current = ussPasadena.GetPos();
            SimEngine::Vec2 next = SimEngine::Math::MoveTowards(current, destination, speed * dt);
            ussPasadena.SetPos(next);
            
            if (SimEngine::Math::Equals(next, destination)) {
                isMoving = false;
            }
        }

        // 3. RENDER (Agnostic)
        SimEngine::Renderer::Begin();
        SimEngine::Renderer::Clear(SimEngine::Colors::RayWhite);
        
        SimEngine::Renderer::DrawCircle(unknownTarget.GetPos(), 10.0f, SimEngine::Colors::Red);
        SimEngine::Vec2 labelunknownTarget = unknownTarget.GetPos();
        labelunknownTarget.x += 15.0f; // Move Right
        labelunknownTarget.y += 20.0f; // Move Down
        SimEngine::Renderer::DrawText("Unknown Target", labelunknownTarget, 20, SimEngine::Colors::Red);

        SimEngine::Renderer::DrawCircle(ussPasadena.GetPos(), 10.0f, SimEngine::Colors::Blue); // Blue
        SimEngine::Vec2 labelussPasadena = ussPasadena.GetPos();
        labelussPasadena.x += 15.0f; // Move Right
        labelussPasadena.y += 20.0f; // Move Down
        SimEngine::Renderer::DrawText("USS Pasadena", labelussPasadena, 20, SimEngine::Colors::Blue);

        if (isMoving) {
            SimEngine::Renderer::DrawCircleLines(destination, 5.0f, 0x00FF00FF); // Green
        }
        SimEngine::Renderer::End();
    }

    SimEngine::Renderer::Close();
    return 0;
}
