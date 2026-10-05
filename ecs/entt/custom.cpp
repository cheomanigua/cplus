#include <iostream>
#include <vector>

struct Position {
    float x{}, y{};
};

struct Registry {
    std::vector<std::size_t> entities{};
    std::vector<Position> positions{};
    void create(float posX, float posY) {
        entities.push_back(entities.size());
        positions.push_back({posX, posY});
    }
    void destroy(std::size_t id) {
        entities.erase(entities.begin() + id);
        positions.erase(positions.begin() + id);
    }
};

void printPos(Registry& reg)
{
    for(std::size_t i = 0; i < reg.entities.size(); ++i) {
        std::cout << "Entity "
            << i << ": "
            << reg.positions[i].x << ", "
            << reg.positions[i].y << "\n";
    }
    std::cout << "\n";
}

int main() {
    Registry reg{};

    for(std::size_t i = 0; i < 5; ++i) {
        reg.create(static_cast<float>(i), static_cast<float>(i));
    }
    printPos(reg);

    reg.destroy(1);
    printPos(reg);
}
