#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

#include "flecs.h"
#include "raylib.h"

constexpr int SCREEN_WIDTH = 1280;
constexpr int SCREEN_HEIGHT = 800;

constexpr float TILE_SIZE = 32.0f;

constexpr int MAP_WIDTH = 40;
constexpr int MAP_HEIGHT = 25;

constexpr std::size_t ENEMY_COUNT = 100;

constexpr float PLAYER_RADIUS = 11.0f;
constexpr float ENEMY_RADIUS = 9.0f;
constexpr float ENEMY_DIAMETER = ENEMY_RADIUS * 2.0f;

struct Position {
    float x{}, y{};
};

struct Velocity {
    float x{}, y{};
};

struct Player {};
struct Enemy {};

struct EnemySnapshot {
    flecs::entity_t id;
    float x;
    float y;
};

struct WallMap {
    bool tiles[MAP_HEIGHT][MAP_WIDTH]{};

    bool solid(int x, int y) const {
        if (x < 0 || x >= MAP_WIDTH ||
            y < 0 || y >= MAP_HEIGHT) {
            return true;
        }

        return tiles[y][x];
    }

    bool blocked(float x, float y, float radius) const {
        const int minX = static_cast<int>(
            std::floor((x - radius) / TILE_SIZE));
        const int maxX = static_cast<int>(
            std::floor((x + radius) / TILE_SIZE));
        const int minY = static_cast<int>(
            std::floor((y - radius) / TILE_SIZE));
        const int maxY = static_cast<int>(
            std::floor((y + radius) / TILE_SIZE));

        for (int ty = minY; ty <= maxY; ++ty) {
            for (int tx = minX; tx <= maxX; ++tx) {
                if (!solid(tx, ty)) {
                    continue;
                }

                const float left = tx * TILE_SIZE;
                const float top = ty * TILE_SIZE;
                const float right = left + TILE_SIZE;
                const float bottom = top + TILE_SIZE;

                const float closestX = std::clamp(x, left, right);
                const float closestY = std::clamp(y, top, bottom);

                const float dx = x - closestX;
                const float dy = y - closestY;

                if (dx * dx + dy * dy < radius * radius) {
                    return true;
                }
            }
        }

        return false;
    }
};

// ------------------------------------------------------------
// Dungeon generation
// ------------------------------------------------------------

void carveRoom(WallMap& map, int x, int y, int w, int h) {
    for (int ty = y; ty < y + h; ++ty) {
        for (int tx = x; tx < x + w; ++tx) {
            if (tx > 0 && tx < MAP_WIDTH - 1 &&
                ty > 0 && ty < MAP_HEIGHT - 1) {
                map.tiles[ty][tx] = false;
            }
        }
    }
}

void carveHorizontal(WallMap& map, int x1, int x2, int y) {
    for (int x = std::min(x1, x2);
         x <= std::max(x1, x2); ++x) {
        if (x > 0 && x < MAP_WIDTH - 1 &&
            y > 0 && y < MAP_HEIGHT - 1) {
            map.tiles[y][x] = false;
        }
    }
}

void carveVertical(WallMap& map, int y1, int y2, int x) {
    for (int y = std::min(y1, y2);
         y <= std::max(y1, y2); ++y) {
        if (x > 0 && x < MAP_WIDTH - 1 &&
            y > 0 && y < MAP_HEIGHT - 1) {
            map.tiles[y][x] = false;
        }
    }
}

WallMap createDungeon() {
    WallMap map;

    for (int y = 0; y < MAP_HEIGHT; ++y) {
        for (int x = 0; x < MAP_WIDTH; ++x) {
            map.tiles[y][x] = true;
        }
    }

    // Rooms.
    carveRoom(map, 2, 2, 10, 7);
    carveRoom(map, 15, 2, 10, 7);
    carveRoom(map, 29, 2, 9, 7);

    carveRoom(map, 3, 12, 11, 10);
    carveRoom(map, 17, 12, 8, 10);
    carveRoom(map, 29, 12, 9, 10);

    // Upper corridors.
    carveHorizontal(map, 11, 15, 5);
    carveHorizontal(map, 24, 29, 5);

    // Vertical connections.
    carveVertical(map, 8, 12, 7);
    carveVertical(map, 8, 12, 20);
    carveVertical(map, 8, 12, 33);

    // Lower corridors.
    carveHorizontal(map, 13, 17, 16);
    carveHorizontal(map, 24, 29, 16);

    carveVertical(map, 16, 22, 7);
    carveVertical(map, 16, 22, 20);
    carveVertical(map, 16, 22, 33);

    return map;
}

// ------------------------------------------------------------
// Collision helpers
// ------------------------------------------------------------

void moveWithWallCollision(
    Position& pos,
    float dx,
    float dy,
    float radius,
    const WallMap& map)
{
    // Resolve axes independently to allow wall sliding.
    if (!map.blocked(pos.x + dx, pos.y, radius)) {
        pos.x += dx;
    }

    if (!map.blocked(pos.x, pos.y + dy, radius)) {
        pos.y += dy;
    }
}

bool overlapsEnemy(
    float x,
    float y,
    flecs::entity_t self,
    const std::vector<EnemySnapshot>& enemies)
{
    for (const auto& other : enemies) {
        if (other.id == self) {
            continue;
        }

        const float dx = x - other.x;
        const float dy = y - other.y;

        if (dx * dx + dy * dy <
            ENEMY_DIAMETER * ENEMY_DIAMETER) {
            return true;
        }
    }

    return false;
}

// ------------------------------------------------------------
// Player update
// ------------------------------------------------------------

void updatePlayer(
    flecs::query<Player, Position>& query,
    float dt,
    const WallMap& map)
{
    float dx = 0.0f;
    float dy = 0.0f;

    if (IsKeyDown(KEY_A)) dx -= 1.0f;
    if (IsKeyDown(KEY_D)) dx += 1.0f;
    if (IsKeyDown(KEY_W)) dy -= 1.0f;
    if (IsKeyDown(KEY_S)) dy += 1.0f;

    const float length = std::sqrt(dx * dx + dy * dy);

    if (length > 0.0f) {
        dx /= length;
        dy /= length;
    }

    constexpr float PLAYER_SPEED = 190.0f;

    query.each([&](Player, Position& pos) {
        moveWithWallCollision(
            pos,
            dx * PLAYER_SPEED * dt,
            dy * PLAYER_SPEED * dt,
            PLAYER_RADIUS,
            map
        );
    });
}

// ------------------------------------------------------------
// Enemy update and collision
// ------------------------------------------------------------

void updateEnemies(
    flecs::query<Enemy, Position, Velocity>& query,
    const Position& player,
    float dt,
    const WallMap& map)
{
    constexpr float ENEMY_SPEED = 75.0f;

    // Snapshot enemy positions before processing movement.
    std::vector<EnemySnapshot> enemies;
    enemies.reserve(ENEMY_COUNT);

    query.each([&](
        flecs::entity e,
        Enemy,
        const Position& pos,
        const Velocity&)
    {
        enemies.push_back({
            e.id(),
            pos.x,
            pos.y
        });
    });

    query.each([&](
        flecs::entity e,
        Enemy,
        Position& pos,
        Velocity& vel)
    {
        float dx = player.x - pos.x;
        float dy = player.y - pos.y;

        const float distance = std::sqrt(dx * dx + dy * dy);

        if (distance <= 1.0f) {
            vel.x = 0.0f;
            vel.y = 0.0f;
            return;
        }

        dx /= distance;
        dy /= distance;

        vel.x = dx * ENEMY_SPEED;
        vel.y = dy * ENEMY_SPEED;

        const flecs::entity_t id = e.id();

        // Attempt horizontal movement.
        const float nextX = pos.x + vel.x * dt;

        if (!map.blocked(nextX, pos.y, ENEMY_RADIUS) &&
            !overlapsEnemy(nextX, pos.y, id, enemies)) {
            pos.x = nextX;
        }

        // Attempt vertical movement using the updated X position.
        const float nextY = pos.y + vel.y * dt;

        if (!map.blocked(pos.x, nextY, ENEMY_RADIUS) &&
            !overlapsEnemy(pos.x, nextY, id, enemies)) {
            pos.y = nextY;
        }

        // Refresh this enemy's position for subsequent enemies.
        for (auto& snapshot : enemies) {
            if (snapshot.id == id) {
                snapshot.x = pos.x;
                snapshot.y = pos.y;
                break;
            }
        }
    });
}

// ------------------------------------------------------------
// Rendering
// ------------------------------------------------------------

void drawDungeon(const WallMap& map) {
    for (int y = 0; y < MAP_HEIGHT; ++y) {
        for (int x = 0; x < MAP_WIDTH; ++x) {
            const int px = static_cast<int>(x * TILE_SIZE);
            const int py = static_cast<int>(y * TILE_SIZE);
            const int size = static_cast<int>(TILE_SIZE);

            if (map.tiles[y][x]) {
                DrawRectangle(
                    px, py, size, size,
                    Color{65, 65, 76, 255}
                );

                DrawRectangleLines(
                    px, py, size, size,
                    Color{83, 83, 94, 255}
                );
            } else {
                DrawRectangle(
                    px, py, size, size,
                    Color{35, 31, 32, 255}
                );

                DrawLine(
                    px, py + size,
                    px + size, py + size,
                    Color{43, 38, 38, 255}
                );
            }
        }
    }
}

void drawPlayer(flecs::query<Player, Position>& query) {
    query.each([](Player, const Position& pos) {
        DrawCircleV({pos.x, pos.y}, 13.0f, GOLD);
        DrawCircleV({pos.x, pos.y}, 8.0f, ORANGE);
        DrawCircleV({pos.x + 5.0f, pos.y - 3.0f}, 3.0f, WHITE);
    });
}

void drawEnemies(flecs::query<Enemy, Position>& query) {
    query.each([](Enemy, const Position& pos) {
        DrawCircleV({pos.x, pos.y}, 11.0f, MAROON);
        DrawCircleV({pos.x, pos.y}, 7.0f, RED);
        DrawCircleV({pos.x - 2.0f, pos.y - 2.0f}, 2.0f, YELLOW);
    });
}

// ------------------------------------------------------------
// Main
// ------------------------------------------------------------

int main() {
    InitWindow(
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        "Dungeon Mob - Flecs + raylib"
    );

    SetTargetFPS(60);

    WallMap map = createDungeon();
    flecs::world world;

    constexpr float PLAYER_X = 5.0f * TILE_SIZE;
    constexpr float PLAYER_Y = 5.0f * TILE_SIZE;

    world.entity("Player")
        .add<Player>()
        .set<Position>({PLAYER_X, PLAYER_Y});

    // Spawn enemies on walkable tiles, away from the player.
    for (std::size_t i = 0; i < ENEMY_COUNT; ++i) {
        for (int attempt = 0; attempt < 1000; ++attempt) {
            const int tx = GetRandomValue(1, MAP_WIDTH - 2);
            const int ty = GetRandomValue(1, MAP_HEIGHT - 2);

            if (map.solid(tx, ty)) {
                continue;
            }

            const float x = (tx + 0.5f) * TILE_SIZE;
            const float y = (ty + 0.5f) * TILE_SIZE;

            const float dx = x - PLAYER_X;
            const float dy = y - PLAYER_Y;

            if (dx * dx + dy * dy <= 180.0f * 180.0f) {
                continue;
            }

            world.entity()
                .add<Enemy>()
                .set<Position>({x, y})
                .set<Velocity>({0.0f, 0.0f});

            break;
        }
    }

    // Create cached queries once.
    auto playerQuery = world
        .query_builder<Player, Position>()
        .cached()
        .build();

    auto enemyUpdateQuery = world
        .query_builder<Enemy, Position, Velocity>()
        .cached()
        .build();

    auto enemyDrawQuery = world
        .query_builder<Enemy, Position>()
        .cached()
        .build();

    Position playerPosition{PLAYER_X, PLAYER_Y};

    while (!WindowShouldClose()) {
        const float dt = std::min(GetFrameTime(), 0.05f);

        updatePlayer(playerQuery, dt, map);

        playerQuery.each([&](
            Player,
            const Position& pos)
        {
            playerPosition = pos;
        });

        updateEnemies(
            enemyUpdateQuery,
            playerPosition,
            dt,
            map
        );

        BeginDrawing();
        ClearBackground(BLACK);

        drawDungeon(map);
        drawEnemies(enemyDrawQuery);
        drawPlayer(playerQuery);

        DrawRectangle(
            0,
            SCREEN_HEIGHT - 32,
            SCREEN_WIDTH,
            32,
            Color{0, 0, 0, 190}
        );

        DrawText(
            "WASD: Move",
            12, SCREEN_HEIGHT - 25,
            18, RAYWHITE
        );

        DrawText(
            "SURVIVE THE MOB",
            180, SCREEN_HEIGHT - 25,
            18, RED
        );

        DrawFPS(SCREEN_WIDTH - 90, 10);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
