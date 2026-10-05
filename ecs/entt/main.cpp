#include <iostream>
#include "entt.hpp"

struct Position {
    float x{}, y{};
};

void printPos(entt::registry& reg)
{
    auto view = reg.view<Position>();
    for(auto [entity, pos] : view.each()) {
        std::cout << "Entity "
            << entt::to_entity(entity) << ": "
            << pos.x << ", "
            << pos.y << "\n";
    }
    std::cout << "\n";
}

int main() {
    entt::registry reg{};

    for(std::size_t i = 0; i < 5; ++i) {
        auto entity = reg.create();

        reg.emplace<Position>(entity,
                static_cast<float>(i),
                static_cast<float>(i)
                );
    }
    printPos(reg);

    reg.destroy(static_cast<entt::entity>(3));
    printPos(reg);

    auto entity = reg.create();
    reg.emplace<Position>(entity, 5.0f, 5.0f);
    printPos(reg);
}
