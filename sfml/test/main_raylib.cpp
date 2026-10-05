#include "raylib.h"
#include <raymath.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <limits>
#include <random>
#include <string>
#include <vector>

// ============================================================
// Configuration
// ============================================================

constexpr int MAP_W = 60;
constexpr int MAP_H = 40;
constexpr int TILE_SIZE = 20;
constexpr int WINDOW_W = MAP_W * TILE_SIZE;
constexpr int WINDOW_H = MAP_H * TILE_SIZE;

constexpr float PLAYER_SPEED = 150.0f;
constexpr float PLAYER_RADIUS = 7.0f;
constexpr float MONK_RADIUS = 6.0f;
constexpr float INTERACT_RANGE = 24.0f;
constexpr float INTERACT_RANGE_SQ = INTERACT_RANGE * INTERACT_RANGE;
constexpr int MAX_MONKS = 8;

// ============================================================
// Colors
// ============================================================

constexpr Color COLOR_BACKGROUND = {18, 18, 18, 255};
constexpr Color COLOR_WALL = {55, 55, 55, 255};
constexpr Color COLOR_FLOOR = {115, 105, 90, 255};
constexpr Color COLOR_COURTYARD = {90, 115, 75, 255};
constexpr Color COLOR_GARDEN = {65, 105, 65, 255};
constexpr Color COLOR_CELLAR = {75, 75, 85, 255};
constexpr Color COLOR_DOOR = {150, 105, 55, 255};
constexpr Color COLOR_STAIR = {130, 100, 160, 255};
constexpr Color COLOR_LABEL = {230, 220, 190, 140};

// ============================================================
// Random Number Generator
// ============================================================

static std::mt19937 rng(static_cast<unsigned int>(time(nullptr)));

int RandomInt(int min, int max)
{
    std::uniform_int_distribution<int> dist(min, max);
    return dist(rng);
}

// ============================================================
// Tiles
// ============================================================

enum class Tile
{
    Wall,
    Floor,
    Courtyard,
    Garden,
    Cellar,
    Door,
    Stair
};

bool IsWalkable(Tile tile)
{
    return tile == Tile::Floor || tile == Tile::Courtyard || tile == Tile::Garden ||
           tile == Tile::Cellar || tile == Tile::Door || tile == Tile::Stair;
}

Color GetTileColor(Tile tile)
{
    switch (tile)
    {
        case Tile::Wall: return COLOR_WALL;
        case Tile::Floor: return COLOR_FLOOR;
        case Tile::Courtyard: return COLOR_COURTYARD;
        case Tile::Garden: return COLOR_GARDEN;
        case Tile::Cellar: return COLOR_CELLAR;
        case Tile::Door: return COLOR_DOOR;
        case Tile::Stair: return COLOR_STAIR;
    }

    return COLOR_WALL;
}

// ============================================================
// Room Types
// ============================================================

enum class RoomType
{
    Cloister,
    Church,
    ChapterHouse,
    Refectory,
    Sacristy,
    Dormitory,
    Calefactorium,
    Kitchen,
    Pantry,
    Cellar,
    Scriptorium,
    Library,
    Infirmary,
    Crypt,
    Cemetery,
    Gatehouse,
    Storage,
    Workshop,
    Garden
};

std::string RoomTypeName(RoomType type)
{
    switch (type)
    {
        case RoomType::Cloister: return "CLOISTER";
        case RoomType::Church: return "CHURCH";
        case RoomType::ChapterHouse: return "CHAPTER HOUSE";
        case RoomType::Refectory: return "REFECTORY";
        case RoomType::Sacristy: return "SACRISTY";
        case RoomType::Dormitory: return "DORMITORY";
        case RoomType::Calefactorium: return "CALEFACTORIUM";
        case RoomType::Kitchen: return "KITCHEN";
        case RoomType::Pantry: return "PANTRY";
        case RoomType::Cellar: return "CELLAR";
        case RoomType::Scriptorium: return "SCRIPTORIUM";
        case RoomType::Library: return "LIBRARY";
        case RoomType::Infirmary: return "INFIRMARY";
        case RoomType::Crypt: return "CRYPT";
        case RoomType::Cemetery: return "CEMETERY";
        case RoomType::Gatehouse: return "GATEHOUSE";
        case RoomType::Storage: return "STORAGE";
        case RoomType::Workshop: return "WORKSHOP";
        case RoomType::Garden: return "GARDEN";
    }

    return "ROOM";
}

// ============================================================
// Room
// ============================================================

struct Room
{
    RoomType type;
    int x;
    int y;
    int width;
    int height;

    int CenterX() const { return x + width / 2; }
    int CenterY() const { return y + height / 2; }

    bool Contains(int px, int py) const
    {
        return px >= x && py >= y && px < x + width && py < y + height;
    }
};

// ============================================================
// Cached Room Label
// ============================================================

struct RoomLabel
{
    std::string text;
    Vector2 position;
};

// ============================================================
// Monk
// ============================================================

struct Monk
{
    int id;
    std::string name;
    std::string role;
    std::string personality;
    Vector2 position;
    int roomIndex = -1;
    Color color;
    std::array<int, MAX_MONKS> relationships{};
};

// ============================================================
// Abbey Generator
// ============================================================

class Abbey
{
public:
    std::vector<std::vector<Tile>> tiles;
    std::vector<Room> rooms;
    int entranceRoom = -1;
    int libraryRoom = -1;

    Abbey() { Generate(); }

    void Generate()
    {
        tiles.assign(MAP_H, std::vector<Tile>(MAP_W, Tile::Wall));
        rooms.clear();
        entranceRoom = -1;
        libraryRoom = -1;

        GenerateAbbeyLayout();
        ConnectRooms();
        AddArchitecture();
        AddEntrance();
        AddCellarStairs();
        RestoreOuterWalls();
    }

    bool Overlaps(const Room& a, const Room& b, int padding = 1) const
    {
        return !(a.x + a.width + padding <= b.x || b.x + b.width + padding <= a.x ||
                 a.y + a.height + padding <= b.y || b.y + b.height + padding <= a.y);
    }

    void GenerateAbbeyLayout()
    {
        Room cloister{RoomType::Cloister, (MAP_W - 13) / 2, (MAP_H - 11) / 2, 13, 11};
        rooms.push_back(cloister);

        std::vector<RoomType> quadRooms = {RoomType::Church, RoomType::ChapterHouse, RoomType::Refectory};
        std::vector<int> sides = {0, 1, 2, 3};
        std::shuffle(sides.begin(), sides.end(), rng);

        for (size_t i = 0; i < quadRooms.size(); ++i)
        {
            Room r{};
            r.type = quadRooms[i];

            if (r.type == RoomType::Church) { r.width = 11; r.height = 7; }
            else if (r.type == RoomType::ChapterHouse) { r.width = 7; r.height = 6; }
            else { r.width = 9; r.height = 6; }

            AttachRoomToSide(r, cloister, sides[i]);
            rooms.push_back(r);
        }

        std::vector<RoomType> peripheralTypes = {
            RoomType::Gatehouse, RoomType::Library, RoomType::Scriptorium,
            RoomType::Cellar, RoomType::Pantry, RoomType::Kitchen,
            RoomType::Dormitory, RoomType::Calefactorium, RoomType::Infirmary,
            RoomType::Workshop, RoomType::Storage, RoomType::Garden, RoomType::Cemetery
        };

        std::shuffle(peripheralTypes.begin(), peripheralTypes.end(), rng);

        for (RoomType type : peripheralTypes)
        {
            Room room{};
            room.type = type;
            room.width = RandomInt(5, 8);
            room.height = RandomInt(4, 7);

            if (type == RoomType::Garden || type == RoomType::Cemetery)
            {
                room.width = RandomInt(7, 10);
                room.height = RandomInt(6, 8);
            }

            for (int attempt = 0; attempt < 300; ++attempt)
            {
                room.x = RandomInt(2, MAP_W - room.width - 3);
                room.y = RandomInt(2, MAP_H - room.height - 3);

                bool valid = true;

                for (const Room& existing : rooms)
                {
                    if (Overlaps(room, existing, 1))
                    {
                        valid = false;
                        break;
                    }
                }

                if (valid)
                {
                    rooms.push_back(room);
                    break;
                }
            }
        }

        FindSpecialRooms();
    }

    void AttachRoomToSide(Room& r, const Room& base, int side)
    {
        switch (side)
        {
            case 0: r.x = base.CenterX() - r.width / 2; r.y = base.y - r.height - 1; break;
            case 1: r.x = base.x + base.width + 1; r.y = base.CenterY() - r.height / 2; break;
            case 2: r.x = base.CenterX() - r.width / 2; r.y = base.y + base.height + 1; break;
            case 3: r.x = base.x - r.width - 1; r.y = base.CenterY() - r.height / 2; break;
        }

        r.x = std::clamp(r.x, 2, MAP_W - r.width - 3);
        r.y = std::clamp(r.y, 2, MAP_H - r.height - 3);
    }

    void FindSpecialRooms()
    {
        for (int i = 0; i < static_cast<int>(rooms.size()); ++i)
        {
            if (rooms[i].type == RoomType::Gatehouse) entranceRoom = i;
            if (rooms[i].type == RoomType::Library) libraryRoom = i;
        }
    }

    void CarveRoom(const Room& room)
    {
        Tile floorType = Tile::Floor;

        if (room.type == RoomType::Cloister) floorType = Tile::Courtyard;
        else if (room.type == RoomType::Garden || room.type == RoomType::Cemetery) floorType = Tile::Garden;
        else if (room.type == RoomType::Cellar || room.type == RoomType::Crypt) floorType = Tile::Cellar;

        for (int y = room.y; y < room.y + room.height; ++y)
            for (int x = room.x; x < room.x + room.width; ++x)
                if (x > 0 && y > 0 && x < MAP_W - 1 && y < MAP_H - 1) tiles[y][x] = floorType;
    }

    void CarveCorridor(int x1, int y1, int x2, int y2)
    {
        int x = x1;
        int y = y1;

        while (x != x2)
        {
            CarveTile(x, y);
            x += x2 > x ? 1 : -1;
        }

        while (y != y2)
        {
            CarveTile(x, y);
            y += y2 > y ? 1 : -1;
        }

        CarveTile(x2, y2);
    }

    void CarveTile(int x, int y)
    {
        if (x <= 0 || y <= 0 || x >= MAP_W - 1 || y >= MAP_H - 1) return;
        if (tiles[y][x] == Tile::Wall) tiles[y][x] = Tile::Floor;
    }

    void ConnectRooms()
    {
        if (rooms.empty()) return;

        std::vector<bool> connected(rooms.size(), false);
        connected[0] = true;
        int connectedCount = 1;

        while (connectedCount < static_cast<int>(rooms.size()))
        {
            int bestFrom = -1;
            int bestTo = -1;
            int bestDistance = std::numeric_limits<int>::max();

            for (size_t i = 0; i < rooms.size(); ++i)
            {
                if (!connected[i]) continue;

                for (size_t j = 0; j < rooms.size(); ++j)
                {
                    if (connected[j]) continue;

                    int dist = std::abs(rooms[i].CenterX() - rooms[j].CenterX()) +
                               std::abs(rooms[i].CenterY() - rooms[j].CenterY());

                    if (dist < bestDistance)
                    {
                        bestDistance = dist;
                        bestFrom = static_cast<int>(i);
                        bestTo = static_cast<int>(j);
                    }
                }
            }

            if (bestFrom == -1 || bestTo == -1) break;

            CarveCorridor(rooms[bestFrom].CenterX(), rooms[bestFrom].CenterY(),
                          rooms[bestTo].CenterX(), rooms[bestTo].CenterY());

            connected[bestTo] = true;
            ++connectedCount;
        }
    }

    void AddArchitecture()
    {
        for (const Room& room : rooms) CarveRoom(room);
        AddCloisterDetails();
    }

    void AddCloisterDetails()
    {
        for (const Room& room : rooms)
        {
            if (room.type != RoomType::Cloister) continue;

            int innerX = room.x + 2;
            int innerY = room.y + 2;
            int innerW = room.width - 4;
            int innerH = room.height - 4;

            for (int y = innerY; y < innerY + innerH; ++y)
                for (int x = innerX; x < innerX + innerW; ++x)
                    tiles[y][x] = Tile::Garden;

            for (int x = room.x + 1; x < room.x + room.width - 1; ++x)
            {
                tiles[room.y + 1][x] = Tile::Floor;
                tiles[room.y + room.height - 2][x] = Tile::Floor;
            }

            for (int y = room.y + 1; y < room.y + room.height - 1; ++y)
            {
                tiles[y][room.x + 1] = Tile::Floor;
                tiles[y][room.x + room.width - 2] = Tile::Floor;
            }
        }
    }

    void AddEntrance()
    {
        if (entranceRoom < 0) return;

        const Room& gate = rooms[entranceRoom];
        tiles[gate.CenterY()][gate.x] = Tile::Door;
    }

    void AddCellarStairs()
    {
        for (const Room& room : rooms)
        {
            if (room.type == RoomType::Cellar)
            {
                tiles[room.CenterY()][room.CenterX()] = Tile::Stair;
                return;
            }
        }
    }

    void RestoreOuterWalls()
    {
        for (int x = 0; x < MAP_W; ++x)
        {
            tiles[0][x] = Tile::Wall;
            tiles[MAP_H - 1][x] = Tile::Wall;
        }

        for (int y = 0; y < MAP_H; ++y)
        {
            tiles[y][0] = Tile::Wall;
            tiles[y][MAP_W - 1] = Tile::Wall;
        }
    }

    bool IsWalkableTile(int x, int y) const
    {
        if (x < 0 || y < 0 || x >= MAP_W || y >= MAP_H) return false;
        return IsWalkable(tiles[y][x]);
    }

    bool CircleCollides(Vector2 position, float radius) const
    {
        int minX = std::max(0, static_cast<int>((position.x - radius) / TILE_SIZE));
        int maxX = std::min(MAP_W - 1, static_cast<int>((position.x + radius) / TILE_SIZE));
        int minY = std::max(0, static_cast<int>((position.y - radius) / TILE_SIZE));
        int maxY = std::min(MAP_H - 1, static_cast<int>((position.y + radius) / TILE_SIZE));

        for (int y = minY; y <= maxY; ++y)
        {
            for (int x = minX; x <= maxX; ++x)
            {
                if (IsWalkableTile(x, y)) continue;

                float left = static_cast<float>(x * TILE_SIZE);
                float top = static_cast<float>(y * TILE_SIZE);
                float closestX = std::clamp(position.x, left, left + TILE_SIZE);
                float closestY = std::clamp(position.y, top, top + TILE_SIZE);
                float dx = position.x - closestX;
                float dy = position.y - closestY;

                if (dx * dx + dy * dy < radius * radius) return true;
            }
        }

        return false;
    }
};

// ============================================================
// Static Map Rendering
// ============================================================

void RebuildMapTexture(const Abbey& abbey, RenderTexture2D& target)
{
    BeginTextureMode(target);
    ClearBackground(COLOR_BACKGROUND);

    for (int y = 0; y < MAP_H; ++y)
    {
        for (int x = 0; x < MAP_W; ++x)
        {
            DrawRectangle(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE - 1, TILE_SIZE - 1, GetTileColor(abbey.tiles[y][x]));
        }
    }

    EndTextureMode();
}

void RebuildRoomLabels(const Abbey& abbey, RenderTexture2D& target)
{
    BeginTextureMode(target);

    for (const Room& room : abbey.rooms)
    {
        std::string label = RoomTypeName(room.type);
        int textWidth = MeasureText(label.c_str(), 10);
        int textX = room.CenterX() * TILE_SIZE - textWidth / 2;
        int textY = room.CenterY() * TILE_SIZE - 5;
        DrawText(label.c_str(), textX, textY, 10, COLOR_LABEL);
    }

    EndTextureMode();
}

void RebuildStaticScene(const Abbey& abbey, RenderTexture2D& target)
{
    BeginTextureMode(target);
    ClearBackground(COLOR_BACKGROUND);

    for (int y = 0; y < MAP_H; ++y)
        for (int x = 0; x < MAP_W; ++x)
            DrawRectangle(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE - 1, TILE_SIZE - 1, GetTileColor(abbey.tiles[y][x]));

    for (const Room& room : abbey.rooms)
    {
        std::string label = RoomTypeName(room.type);
        int textWidth = MeasureText(label.c_str(), 10);
        DrawText(label.c_str(), room.CenterX() * TILE_SIZE - textWidth / 2, room.CenterY() * TILE_SIZE - 5, 10, COLOR_LABEL);
    }

    EndTextureMode();
}

// ============================================================
// Main
// ============================================================

int main()
{
    InitWindow(WINDOW_W, WINDOW_H, "Abbey Mystery Investigation Engine (Raylib)");
    SetTargetFPS(60);

    Abbey abbey;
    RenderTexture2D staticScene = LoadRenderTexture(WINDOW_W, WINDOW_H);

    Vector2 playerPosition{};
    std::vector<Monk> monks;
    const Monk* inspectedMonk = nullptr;

    const std::array<std::string, MAX_MONKS> monkNames = {
        "Brother William", "Abbot Abbo", "Brother Severinus", "Brother Malachi",
        "Brother Berengar", "Brother Jorge", "Brother Nicholas", "Brother Adelmo"
    };

    const std::array<std::string, MAX_MONKS> monkRoles = {
        "Investigator", "Abbot", "Herbalist", "Librarian",
        "Assistant Librarian", "Venerable Elder", "Craftsman", "Illuminator"
    };

    const std::array<std::string, MAX_MONKS> monkPersonalities = {
        "Analytical & Calm", "Strict & Authoritative", "Secretive & Knowledgeable", "Anxious & Obsessive",
        "Sly & Jealous", "Dogmatic & Severe", "Practical & Observant", "Nervous & Talented"
    };

    const std::array<Color, MAX_MONKS> monkColors = {
        Color{100, 200, 255, 255}, Color{240, 200, 80, 255},
        Color{100, 220, 100, 255}, Color{180, 100, 220, 255},
        Color{220, 120, 120, 255}, Color{180, 180, 180, 255},
        Color{210, 150, 90, 255}, Color{240, 140, 200, 255}
    };

    auto SpawnGame = [&]()
    {
        abbey.Generate();
        monks.clear();
        inspectedMonk = nullptr;

        if (abbey.entranceRoom >= 0)
        {
            const Room& gate = abbey.rooms[abbey.entranceRoom];
            playerPosition = {(gate.CenterX() + 0.5f) * TILE_SIZE, (gate.CenterY() + 0.5f) * TILE_SIZE};
        }
        else
        {
            playerPosition = {TILE_SIZE * 2.5f, TILE_SIZE * 2.5f};
        }

        int numMonks = std::min<int>(MAX_MONKS, abbey.rooms.size());
        monks.reserve(numMonks);

        for (int i = 0; i < numMonks; ++i)
        {
            Monk monk;
            monk.id = i;
            monk.name = monkNames[i];
            monk.role = monkRoles[i];
            monk.personality = monkPersonalities[i];
            monk.color = monkColors[i];
            monk.roomIndex = i;

            const Room& room = abbey.rooms[i];
            monk.position = {(room.CenterX() + 0.5f) * TILE_SIZE, (room.CenterY() + 0.5f) * TILE_SIZE};

            monks.push_back(std::move(monk));
        }

        for (auto& monk : monks)
            for (const auto& other : monks)
                if (monk.id != other.id) monk.relationships[other.id] = RandomInt(-80, 95);

        RebuildStaticScene(abbey, staticScene);
    };

    SpawnGame();

    while (!WindowShouldClose())
    {
        float dt = std::min(GetFrameTime(), 0.05f);

        if (IsKeyPressed(KEY_R)) SpawnGame();

        Vector2 direction{};

        if (IsKeyDown(KEY_W)) direction.y -= 1.0f;
        if (IsKeyDown(KEY_S)) direction.y += 1.0f;
        if (IsKeyDown(KEY_A)) direction.x -= 1.0f;
        if (IsKeyDown(KEY_D)) direction.x += 1.0f;

        float lengthSq = direction.x * direction.x + direction.y * direction.y;

        if (lengthSq > 0.0f)
        {
            float invLength = 1.0f / std::sqrt(lengthSq);
            direction.x *= invLength;
            direction.y *= invLength;
        }

        Vector2 velocity = Vector2Scale(direction, PLAYER_SPEED * dt);

        Vector2 newX{playerPosition.x + velocity.x, playerPosition.y};
        if (!abbey.CircleCollides(newX, PLAYER_RADIUS)) playerPosition.x = newX.x;

        Vector2 newY{playerPosition.x, playerPosition.y + velocity.y};
        if (!abbey.CircleCollides(newY, PLAYER_RADIUS)) playerPosition.y = newY.y;

        inspectedMonk = nullptr;

        for (const auto& monk : monks)
        {
            float dx = playerPosition.x - monk.position.x;
            float dy = playerPosition.y - monk.position.y;

            if (dx * dx + dy * dy <= INTERACT_RANGE_SQ)
            {
                inspectedMonk = &monk;
                break;
            }
        }

        BeginDrawing();
        ClearBackground(COLOR_BACKGROUND);

        DrawTextureRec(
            staticScene.texture,
            Rectangle{0, 0, static_cast<float>(staticScene.texture.width), -static_cast<float>(staticScene.texture.height)},
            Vector2{0, 0},
            WHITE
        );

        for (const auto& monk : monks)
            DrawCircleV(monk.position, MONK_RADIUS, monk.color);

        DrawCircleV(playerPosition, PLAYER_RADIUS, WHITE);
        DrawCircleLinesV(playerPosition, PLAYER_RADIUS, BLACK);

        DrawText("WASD: Move | R: New Monastery Layout", 8, WINDOW_H - 18, 12, WHITE);

        if (inspectedMonk)
        {
            int panelX = WINDOW_W - 330;
            int panelY = 10;

            DrawRectangle(panelX, panelY, 320, 180, Color{20, 20, 25, 230});
            DrawRectangleLines(panelX, panelY, 320, 180, inspectedMonk->color);

            int startY = panelY + 10;

            DrawText(inspectedMonk->name.c_str(), panelX + 10, startY, 16, inspectedMonk->color);
            DrawText(("Role: " + inspectedMonk->role).c_str(), panelX + 10, startY + 22, 12, WHITE);
            DrawText(("Trait: " + inspectedMonk->personality).c_str(), panelX + 10, startY + 38, 12, Color{200, 200, 200, 255});
            DrawText("Relationships / Opinions:", panelX + 10, startY + 62, 12, Color{240, 200, 100, 255});

            int relY = startY + 78;
            int drawn = 0;

            for (const auto& other : monks)
            {
                if (other.id == inspectedMonk->id || drawn >= 5) continue;

                int opinion = inspectedMonk->relationships[other.id];
                Color opColor = opinion >= 0 ? Color{120, 220, 120, 255} : Color{240, 100, 100, 255};

                std::string line = other.name.substr(0, 16) + ": " + (opinion > 0 ? "+" : "") + std::to_string(opinion);
                DrawText(line.c_str(), panelX + 10, relY, 10, opColor);

                relY += 15;
                ++drawn;
            }
        }

        EndDrawing();
    }

    UnloadRenderTexture(staticScene);
    CloseWindow();

    return 0;
}

