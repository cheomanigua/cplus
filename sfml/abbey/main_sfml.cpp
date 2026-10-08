#include <SFML/Graphics.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <limits>
#include <random>
#include <string>
#include <unordered_map>
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

// ============================================================
// Random Number Generator
// ============================================================

static std::mt19937 rng(
    static_cast<unsigned int>(
        std::chrono::high_resolution_clock::now()
            .time_since_epoch()
            .count()
    )
);

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
    return tile == Tile::Floor ||
           tile == Tile::Courtyard ||
           tile == Tile::Garden ||
           tile == Tile::Cellar ||
           tile == Tile::Door ||
           tile == Tile::Stair;
}

sf::Color TileColor(Tile tile)
{
    switch (tile)
    {
        case Tile::Wall:
            return sf::Color(55, 55, 55);

        case Tile::Floor:
            return sf::Color(115, 105, 90);

        case Tile::Courtyard:
            return sf::Color(90, 115, 75);

        case Tile::Garden:
            return sf::Color(65, 105, 65);

        case Tile::Cellar:
            return sf::Color(75, 75, 85);

        case Tile::Door:
            return sf::Color(150, 105, 55);

        case Tile::Stair:
            return sf::Color(130, 100, 160);
    }

    return sf::Color::Magenta;
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
        case RoomType::Cloister:       return "CLOISTER";
        case RoomType::Church:         return "CHURCH";
        case RoomType::ChapterHouse:   return "CHAPTER HOUSE";
        case RoomType::Refectory:      return "REFECTORY";
        case RoomType::Sacristy:       return "SACRISTY";
        case RoomType::Dormitory:      return "DORMITORY";
        case RoomType::Calefactorium:  return "CALEFACTORIUM";
        case RoomType::Kitchen:        return "KITCHEN";
        case RoomType::Pantry:         return "PANTRY";
        case RoomType::Cellar:         return "CELLAR";
        case RoomType::Scriptorium:    return "SCRIPTORIUM";
        case RoomType::Library:        return "LIBRARY";
        case RoomType::Infirmary:      return "INFIRMARY";
        case RoomType::Crypt:          return "CRYPT";
        case RoomType::Cemetery:       return "CEMETERY";
        case RoomType::Gatehouse:      return "GATEHOUSE";
        case RoomType::Storage:        return "STORAGE";
        case RoomType::Workshop:       return "WORKSHOP";
        case RoomType::Garden:         return "GARDEN";
    }

    return "ROOM";
}

// ============================================================
// Room
// ============================================================

struct Room
{
    RoomType type;

    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;

    int CenterX() const
    {
        return x + width / 2;
    }

    int CenterY() const
    {
        return y + height / 2;
    }

    bool Contains(int px, int py) const
    {
        return px >= x &&
               py >= y &&
               px < x + width &&
               py < y + height;
    }
};

// ============================================================
// Monk
// ============================================================

struct Monk
{
    int id = 0;

    std::string name;
    std::string role;
    std::string personality;

    sf::Vector2f position;

    int roomIndex = -1;

    sf::Color color;

    std::unordered_map<int, int> relationships;
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

    Abbey()
    {
        Generate();
    }

    void Generate()
    {
        tiles.assign(
            MAP_H,
            std::vector<Tile>(MAP_W, Tile::Wall)
        );

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

    // --------------------------------------------------------
    // Room overlap
    // --------------------------------------------------------

    bool Overlaps(
        const Room& a,
        const Room& b,
        int padding = 1
    ) const
    {
        return !(
            a.x + a.width + padding <= b.x ||
            b.x + b.width + padding <= a.x ||
            a.y + a.height + padding <= b.y ||
            b.y + b.height + padding <= a.y
        );
    }

    // --------------------------------------------------------
    // Layout
    // --------------------------------------------------------

    void GenerateAbbeyLayout()
    {
        // Central cloister

        Room cloister;

        cloister.type = RoomType::Cloister;
        cloister.width = 13;
        cloister.height = 11;
        cloister.x = (MAP_W - cloister.width) / 2;
        cloister.y = (MAP_H - cloister.height) / 2;

        rooms.push_back(cloister);

        // ----------------------------------------------------
        // Major rooms around cloister
        // ----------------------------------------------------

        std::vector<RoomType> quadRooms =
        {
            RoomType::Church,
            RoomType::ChapterHouse,
            RoomType::Refectory
        };

        std::vector<int> sides =
        {
            0, 1, 2, 3
        };

        std::shuffle(
            sides.begin(),
            sides.end(),
            rng
        );

        for (std::size_t i = 0; i < quadRooms.size(); ++i)
        {
            Room room;

            room.type = quadRooms[i];

            if (room.type == RoomType::Church)
            {
                room.width = 11;
                room.height = 7;
            }
            else if (room.type == RoomType::ChapterHouse)
            {
                room.width = 7;
                room.height = 6;
            }
            else if (room.type == RoomType::Refectory)
            {
                room.width = 9;
                room.height = 6;
            }

            AttachRoomToSide(
                room,
                cloister,
                sides[i]
            );

            rooms.push_back(room);
        }

        // ----------------------------------------------------
        // Peripheral rooms
        // ----------------------------------------------------

        std::vector<RoomType> peripheralTypes =
        {
            RoomType::Gatehouse,
            RoomType::Library,
            RoomType::Scriptorium,
            RoomType::Cellar,
            RoomType::Pantry,
            RoomType::Kitchen,
            RoomType::Dormitory,
            RoomType::Calefactorium,
            RoomType::Infirmary,
            RoomType::Workshop,
            RoomType::Storage,
            RoomType::Garden,
            RoomType::Cemetery
        };

        std::shuffle(peripheralTypes.begin(), peripheralTypes.end(), rng);

        for (RoomType type : peripheralTypes)
        {
            Room room;
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

    // --------------------------------------------------------
    // Attach room
    // --------------------------------------------------------

    void AttachRoomToSide(Room& room, const Room& base, int side)
    {
        switch (side)
        {
            case 0:
                room.x = base.CenterX() - room.width / 2;
                room.y = base.y - room.height - 1;
                break;
        
            case 1:
                room.x = base.x + base.width + 1;
                room.y = base.CenterY() - room.height / 2;
                break;
        
            case 2:
                room.x = base.CenterX() - room.width / 2;
                room.y = base.y + base.height + 1;
                break;
        
            case 3:
                room.x = base.x - room.width - 1;
                room.y = base.CenterY() - room.height / 2;
                break;
        }
        
        room.x = std::clamp(room.x, 2, MAP_W - room.width - 3);
        room.y = std::clamp(room.y, 2, MAP_H - room.height - 3);
    }

    // --------------------------------------------------------
    // Find special rooms
    // --------------------------------------------------------

    void FindSpecialRooms()
    {
        entranceRoom = -1;
        libraryRoom = -1;

        for (int i = 0; i < static_cast<int>(rooms.size()); ++i)
        {
            if (rooms[i].type == RoomType::Gatehouse)
                entranceRoom = i;

            if (rooms[i].type == RoomType::Library)
                libraryRoom = i;
        }
    }

    // --------------------------------------------------------
    // Carve room
    // --------------------------------------------------------

    void CarveRoom(const Room& room)
    {
        Tile floorType = Tile::Floor;

        if (room.type == RoomType::Cloister)
        {
            floorType = Tile::Courtyard;
        }
        else if (
            room.type == RoomType::Garden ||
            room.type == RoomType::Cemetery
        )
        {
            floorType = Tile::Garden;
        }
        else if (
            room.type == RoomType::Cellar ||
            room.type == RoomType::Crypt
        )
        {
            floorType = Tile::Cellar;
        }

        for (int y = room.y; y < room.y + room.height; ++y)
        {
            for (int x = room.x; x < room.x + room.width; ++x)
            {
                if (x > 0 && y > 0 && x < MAP_W - 1 && y < MAP_H - 1)
                {
                    tiles[y][x] = floorType;
                }
            }
        }
    }

    // --------------------------------------------------------
    // Corridor
    // --------------------------------------------------------

    void CarveCorridor(int x1, int y1, int x2, int y2)
    {
        int x = x1;
        int y = y1;

        while (x != x2)
        {
            CarveTile(x, y);
            x += (x2 > x) ? 1 : -1;
        }

        while (y != y2)
        {
            CarveTile(x, y);
            y += (y2 > y) ? 1 : -1;
        }
        CarveTile(x2, y2);
    }

    void CarveTile(int x, int y)
    {
        if (x <= 0 || y <= 0 || x >= MAP_W - 1 || y >= MAP_H - 1)
        {
            return;
        }

        if (tiles[y][x] == Tile::Wall)
        {
            tiles[y][x] = Tile::Floor;
        }
    }

    // --------------------------------------------------------
    // Connect rooms
    // --------------------------------------------------------

    void ConnectRooms()
    {
        if (rooms.empty())
            return;

        std::vector<bool> connected(rooms.size(), false);

        connected[0] = true;

        int connectedCount = 1;

        while (
            connectedCount <
            static_cast<int>(rooms.size())
        )
        {
            int bestFrom = -1;
            int bestTo = -1;

            int bestDistance = std::numeric_limits<int>::max();

            for (std::size_t i = 0; i < rooms.size(); ++i)
            {
                if (!connected[i])
                    continue;

                for (std::size_t j = 0; j < rooms.size(); ++j)
                {
                    if (connected[j])
                        continue;

                    int distance =
                        std::abs(rooms[i].CenterX() - rooms[j].CenterX())
                        +
                        std::abs(rooms[i].CenterY() - rooms[j].CenterY());

                    if (distance < bestDistance)
                    {
                        bestDistance = distance;
                        bestFrom = static_cast<int>(i);
                        bestTo = static_cast<int>(j);
                    }
                }
            }

            if (bestFrom != -1 && bestTo != -1)
            {
                CarveCorridor(
                    rooms[bestFrom].CenterX(),
                    rooms[bestFrom].CenterY(),
                    rooms[bestTo].CenterX(),
                    rooms[bestTo].CenterY()
                );

                connected[bestTo] = true;
                ++connectedCount;
            }
            else
            {
                break;
            }
        }
    }

    // --------------------------------------------------------
    // Architecture
    // --------------------------------------------------------

    void AddArchitecture()
    {
        for (const Room& room : rooms)
            CarveRoom(room);

        AddCloisterDetails();
    }

    void AddCloisterDetails()
    {
        for (const Room& room : rooms)
        {
            if (room.type != RoomType::Cloister)
                continue;

            int innerX = room.x + 2;
            int innerY = room.y + 2;

            int innerW = room.width - 4;
            int innerH = room.height - 4;

            for (int y = innerY; y < innerY + innerH; ++y)
            {
                for (int x = innerX; x < innerX + innerW; ++x)
                    tiles[y][x] = Tile::Garden;
            }

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

    // --------------------------------------------------------
    // Entrance
    // --------------------------------------------------------

    void AddEntrance()
    {
        if (entranceRoom < 0)
            return;

        const Room& gate = rooms[entranceRoom];

        tiles[gate.CenterY()][gate.x] = Tile::Door;
    }

    // --------------------------------------------------------
    // Cellar stairs
    // --------------------------------------------------------

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

    // --------------------------------------------------------
    // Outer walls
    // --------------------------------------------------------

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

    // --------------------------------------------------------
    // Collision
    // --------------------------------------------------------
    
    bool IsWalkableTile(int x, int y) const
    {
        if (x < 0 || y < 0 || x >= MAP_W || y >= MAP_H) return false;
        return IsWalkable(tiles[y][x]);
    }

    bool CircleCollides(sf::Vector2f position, float radius) const
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
// Static Abbey Map
//
// The complete tile map is represented by one vertex array.
// The map is rebuilt only when the abbey is regenerated.
// ============================================================

class AbbeyMapDrawable : public sf::Drawable
{
public:
    void Build(const Abbey& abbey)
    {
        vertices.clear();
        vertices.setPrimitiveType(sf::PrimitiveType::Triangles);
        vertices.resize(MAP_W * MAP_H * 6);

        std::size_t index = 0;

        for (int y = 0; y < MAP_H; ++y)
        {
            for (int x = 0; x < MAP_W; ++x)
            {
                const float left = static_cast<float>(x * TILE_SIZE);
                const float top = static_cast<float>(y * TILE_SIZE);
                const float right = left + TILE_SIZE - 1.0f;
                const float bottom = top + TILE_SIZE - 1.0f;
                const sf::Color color = TileColor(abbey.tiles[y][x]);

                vertices[index++] = sf::Vertex({left, top}, color);
                vertices[index++] = sf::Vertex({right, top}, color);
                vertices[index++] = sf::Vertex({right, bottom}, color);

                vertices[index++] = sf::Vertex({left, top}, color);
                vertices[index++] = sf::Vertex({right, bottom}, color);
                vertices[index++] = sf::Vertex({left, bottom}, color);
            }
        }
    }

private:
    sf::VertexArray vertices;

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        target.draw(vertices, states);
    }
};


// ============================================================
// Main
// ============================================================

int main()
{
    sf::RenderWindow window(
        sf::VideoMode({
            WINDOW_W,
            WINDOW_H
        }),
        "Abbey Mystery Investigation Engine"
    );

    window.setFramerateLimit(60);

    // --------------------------------------------------------
    // Font
    // --------------------------------------------------------

    sf::Font font;

    if (!font.openFromFile("resources/DejaVuSansMono.ttf"))
    {
        return 1;
    }

    // --------------------------------------------------------
    // Game state
    // --------------------------------------------------------

    Abbey abbey;

    AbbeyMapDrawable abbeyMap;

    abbeyMap.Build(abbey);

    sf::Vector2f playerPosition;

    std::vector<Monk> monks;

    const Monk* inspectedMonk = nullptr;

    // --------------------------------------------------------
    // Static monk data
    // --------------------------------------------------------

    const std::vector<std::string> monkNames =
    {
        "Brother William",
        "Abbot Abbo",
        "Brother Severinus",
        "Brother Malachi",
        "Brother Berengar",
        "Brother Jorge",
        "Brother Nicholas",
        "Brother Adelmo"
    };

    const std::vector<std::string> monkRoles =
    {
        "Investigator",
        "Abbot",
        "Herbalist",
        "Librarian",
        "Assistant Librarian",
        "Venerable Elder",
        "Craftsman",
        "Illuminator"
    };

    const std::vector<std::string> monkPersonalities =
    {
        "Analytical & Calm",
        "Strict & Authoritative",
        "Secretive & Knowledgeable",
        "Anxious & Obsessive",
        "Sly & Jealous",
        "Dogmatic & Severe",
        "Practical & Observant",
        "Nervous & Talented"
    };

    const std::vector<sf::Color> monkColors =
    {
        sf::Color(100, 200, 255),
        sf::Color(240, 200, 80),
        sf::Color(100, 220, 100),
        sf::Color(180, 100, 220),
        sf::Color(220, 120, 120),
        sf::Color(180, 180, 180),
        sf::Color(210, 150, 90),
        sf::Color(240, 140, 200)
    };

    // --------------------------------------------------------
    // Reusable shapes
    // --------------------------------------------------------

    sf::CircleShape monkShape(MONK_RADIUS);
    monkShape.setOrigin({MONK_RADIUS, MONK_RADIUS});

    sf::CircleShape playerShape(PLAYER_RADIUS);
    playerShape.setOrigin({PLAYER_RADIUS, PLAYER_RADIUS});
    playerShape.setFillColor(sf::Color::White);
    playerShape.setOutlineThickness(1.5f);
    playerShape.setOutlineColor(sf::Color::Black);

    // --------------------------------------------------------
    // Cached room labels
    // --------------------------------------------------------

    std::vector<sf::Text> roomLabels;

    auto BuildRoomLabels = [&]()
    {
        roomLabels.clear();
        roomLabels.reserve(abbey.rooms.size());

        for (const Room& room : abbey.rooms)
        {
            sf::Text label(font, RoomTypeName(room.type), 8);
            sf::FloatRect bounds = label.getLocalBounds();

            label.setOrigin({
                bounds.position.x + bounds.size.x / 2.0f,
                bounds.position.y + bounds.size.y / 2.0f
            });

            label.setPosition({
                (room.CenterX() + 0.5f) * TILE_SIZE,
                (room.CenterY() + 0.5f) * TILE_SIZE
            });

            label.setFillColor(sf::Color(230, 220, 190, 140));
            roomLabels.push_back(std::move(label));
        }
    };

    BuildRoomLabels();

    // --------------------------------------------------------
    // Cached HUD
    // --------------------------------------------------------

    sf::Text hudText(font, "WASD: Move | R: New Monastery Layout", 12);
    hudText.setPosition({8.0f, WINDOW_H - 18.0f});
    hudText.setFillColor(sf::Color::White);

    // --------------------------------------------------------
    // Inspection panel
    // --------------------------------------------------------

    sf::RectangleShape inspectionPanel(sf::Vector2f(320.0f, 180.0f));
    inspectionPanel.setPosition({WINDOW_W - 330.0f, 10.0f});
    inspectionPanel.setFillColor(sf::Color(20, 20, 25, 230));
    inspectionPanel.setOutlineThickness(2.0f);

    // Cached inspection texts

    sf::Text inspectionName(font, "", 14);
    sf::Text inspectionRole(font, "", 11);
    sf::Text inspectionTrait(font, "", 11);
    sf::Text inspectionRelationships(
        font,
        "Relationships / Opinions:",
        11
    );

    std::vector<sf::Text> relationshipTexts;

    relationshipTexts.reserve(5);

    for (int i = 0; i < 5; ++i)
        relationshipTexts.emplace_back(font, "", 10);

    inspectionRole.setFillColor(sf::Color::White);
    inspectionTrait.setFillColor(sf::Color(200, 200, 200));
    inspectionRelationships.setFillColor(sf::Color(240, 200, 100));

    // --------------------------------------------------------
    // Update inspection UI
    // --------------------------------------------------------

    auto UpdateInspectionUI = [&](const Monk* monk)
    {
        if (!monk) return;

        inspectionPanel.setOutlineColor(monk->color);
        inspectionName.setString(monk->name);
        inspectionName.setFillColor(monk->color);
        inspectionRole.setString("Role: " + monk->role);
        inspectionTrait.setString("Trait: " + monk->personality);

        int drawn = 0;

        for (const Monk& other : monks)
        {
            if (other.id == monk->id || drawn >= 5) continue;

            const auto it = monk->relationships.find(other.id);
            if (it == monk->relationships.end()) continue;

            const int opinion = it->second;
            std::string line = other.name.substr(0, 16) + ": " + (opinion > 0 ? "+" : "") + std::to_string(opinion);

            relationshipTexts[drawn].setString(line);
            relationshipTexts[drawn].setFillColor(opinion >= 0 ? sf::Color(120, 220, 120) : sf::Color(240, 100, 100));

            ++drawn;
        }

        for (int i = drawn; i < 5; ++i)
            relationshipTexts[i].setString("");
    };


    // --------------------------------------------------------
    // Spawn game
    // --------------------------------------------------------

    auto SpawnGame = [&]()
    {
        abbey.Generate();

        // Rebuild static GPU map.
        abbeyMap.Build(abbey);

        // Rebuild room labels.
        BuildRoomLabels();

        monks.clear();

        inspectedMonk = nullptr;

        // ----------------------------------------------------
        // Player spawn
        // ----------------------------------------------------

        if (abbey.entranceRoom >= 0)
        {
            const Room& gate = abbey.rooms[abbey.entranceRoom];
            playerPosition = {(gate.CenterX() + 0.5f) * TILE_SIZE, (gate.CenterY() + 0.5f) * TILE_SIZE};
        }
        else
        {
            playerPosition = {TILE_SIZE * 2.5f, TILE_SIZE * 2.5f};
        }
        
        // ----------------------------------------------------
        // Spawn monks
        // ----------------------------------------------------
        
        const int numMonks = std::min<int>(monkNames.size(), abbey.rooms.size());
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
            monk.position = {(room.CenterX() + 0.5f) * TILE_SIZE,
                            (room.CenterY() + 0.5f) * TILE_SIZE};
        
            monks.push_back(std::move(monk));
        }


        // ----------------------------------------------------
        // Relationships
        // ----------------------------------------------------

        for (auto& monk : monks)
        {
            monk.relationships.reserve(monks.size() - 1);

            for (const auto& other : monks)
            {
                if (monk.id == other.id)
                    continue;

                monk.relationships[other.id] = RandomInt(-80, 95);
            }
        }
    };

    SpawnGame();

    // --------------------------------------------------------
    // Timing
    // --------------------------------------------------------

    sf::Clock clock;

    // ========================================================
    // Main loop
    // ========================================================


    while (window.isOpen())
    {
        float dt = std::min(clock.restart().asSeconds(), 0.05f);

        // ----------------------------------------------------
        // Events
        // ----------------------------------------------------

        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();

            if (const auto* key = event->getIf<sf::Event::KeyPressed>())
            {
                if (key->scancode == sf::Keyboard::Scancode::R)
                    SpawnGame();
            }
        }

        // ----------------------------------------------------
        // Player movement
        // ----------------------------------------------------

        sf::Vector2f direction{0.0f, 0.0f};

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::W)) direction.y -= 1.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::S)) direction.y += 1.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::A)) direction.x -= 1.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::D)) direction.x += 1.0f;

        const float length = std::hypot(direction.x, direction.y);
        if (length > 0.0f) direction /= length;

        const sf::Vector2f velocity = direction * PLAYER_SPEED * dt;

        // X collision

        sf::Vector2f newX{playerPosition.x + velocity.x, playerPosition.y};

        if (!abbey.CircleCollides(newX, PLAYER_RADIUS))
            playerPosition.x = newX.x;

        // Y collision

        sf::Vector2f newY{playerPosition.x, playerPosition.y + velocity.y};

        if (!abbey.CircleCollides(newY, PLAYER_RADIUS))
            playerPosition.y = newY.y;

        // ----------------------------------------------------
        // Monk proximity
        // ----------------------------------------------------

        const Monk* newInspectedMonk = nullptr;

        for (const auto& monk : monks)
        {
            const float dx = playerPosition.x - monk.position.x;
            const float dy = playerPosition.y - monk.position.y;
            const float distanceSquared = dx * dx + dy * dy;

            if (distanceSquared <= INTERACT_RANGE * INTERACT_RANGE)
            {
                newInspectedMonk = &monk;
                break;
            }
        }

        // Only rebuild inspection UI when the selected monk changes.

        if (newInspectedMonk != inspectedMonk)
        {
            inspectedMonk = newInspectedMonk;

            if (inspectedMonk)
                UpdateInspectionUI(inspectedMonk);
        }

        // ====================================================
        // Rendering
        // ====================================================

        window.clear(sf::Color(18, 18, 18));

        // ----------------------------------------------------
        // STATIC MAP
        //
        // One vertex-array draw call.
        // ----------------------------------------------------

        window.draw(abbeyMap);

        // ----------------------------------------------------
        // Room labels
        // ----------------------------------------------------

        for (const auto& label : roomLabels)
        {
            window.draw(label);
        }

        // ----------------------------------------------------
        // Monks
        // ----------------------------------------------------

        for (const auto& monk : monks)
        {
            monkShape.setPosition(monk.position);
            monkShape.setFillColor(monk.color);
            window.draw(monkShape);
        }
        
        // ----------------------------------------------------
        // Player
        // ----------------------------------------------------
        
        playerShape.setPosition(playerPosition);
        window.draw(playerShape);
        
        // ----------------------------------------------------
        // HUD
        // ----------------------------------------------------
        
        window.draw(hudText);
        
        // ----------------------------------------------------
        // Inspection panel
        // ----------------------------------------------------
        
        if (inspectedMonk)
        {
            inspectionPanel.setPosition({WINDOW_W - 330.0f, 10.0f});
            window.draw(inspectionPanel);
        
            const float startY = 18.0f;
        
            inspectionName.setPosition({WINDOW_W - 320.0f, startY});
            inspectionRole.setPosition({WINDOW_W - 320.0f, startY + 20.0f});
            inspectionTrait.setPosition({WINDOW_W - 320.0f, startY + 36.0f});
            inspectionRelationships.setPosition({WINDOW_W - 320.0f, startY + 60.0f});
        
            window.draw(inspectionName);
            window.draw(inspectionRole);
            window.draw(inspectionTrait);
            window.draw(inspectionRelationships);
        
            for (int i = 0; i < 5; ++i)
            {
                relationshipTexts[i].setPosition({WINDOW_W - 320.0f, startY + 76.0f + i * 15.0f});
                window.draw(relationshipTexts[i]);
            }
        }

        // ----------------------------------------------------
        // Present
        // ----------------------------------------------------

        window.display();
    }

    return 0;
}

