#include "raylib.h"
#include "raymath.h"
#include <array>
#include <iostream>

constexpr std::size_t MaxBullets { 128 };

struct Player {
    Vector2 position{};
    float radius{};
    float speed{};
};

struct Bullet
{
    Vector2 position{};
    Vector2 direction{};
    float speed { 1000.0f };
    float radius { 2.0f };
    float lifetime { 2.0f };
    bool active { false };
};


Vector2 GetMovementDirection() {
    Vector2 direction = {
        static_cast<float>(IsKeyDown(KEY_D)) - static_cast<float>(IsKeyDown(KEY_A)),
        static_cast<float>(IsKeyDown(KEY_S))  - static_cast<float>(IsKeyDown(KEY_W))
    };

    return Vector2Length(direction) > 0.0f ? Vector2Normalize(direction) : Vector2{0.0f, 0.0f};
}

void Shoot(std::array<Bullet, MaxBullets>& bullets, const Player& player)
{
    for (Bullet& bullet : bullets)
    {
        if (bullet.active)
            continue;

        bullet.position = player.position;
        bullet.direction = Vector2Normalize(Vector2Subtract(GetMousePosition(), player.position));
        bullet.lifetime = 2.0f;
        bullet.active = true;

        return;     // only spawn one bullet

    }
}

void UpdateBullets(std::array<Bullet, MaxBullets>& bullets, float dt)
{
    for (Bullet& bullet : bullets)
        {
        if (!bullet.active)
            continue;

        bullet.position = Vector2Add(bullet.position, Vector2Scale(bullet.direction, bullet.speed * dt));

        bullet.lifetime -= dt;

        if (bullet.lifetime <= 0.0f)
        {
            bullet.active = false;
        }
    }
}

void DrawBullets(const std::array<Bullet, MaxBullets>& bullets)
{
    for (const Bullet& bullet : bullets)
    {
        if (bullet.active)
            DrawCircleV(bullet.position, bullet.radius, BLUE);
    }
}

int CountActiveBullets(const std::array<Bullet, MaxBullets>& bullets)
{
    int count = 0;

    for (const Bullet& bullet : bullets)
    {
        if (bullet.active)
            ++count;
    }

    return count;
}

int main()
{
    constexpr int screenWidth { 800 };
    constexpr int screenHeight { 450 };

    Player player { {400.0f, 215.0f}, 20.0f, 100.0f };
    Item potion {1, "Health Potion", {100, 100}, {5.0f}, RED, Trigger::OnUse, Execution::Inmediate, Targeting::Self, false, Persistence::Instant, Attribute::Health};
    printItem(potion);
    std::array<Bullet, MaxBullets> bullets;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - input keys");
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            Shoot(bullets, player);
        }

        UpdateBullets(bullets, dt);

        // Move
        player.position = Vector2Add(player.position, Vector2Scale(GetMovementDirection(), player.speed * dt));

        // Draw
        BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawCircleV(player.position, player.radius, BLUE);
            DrawCircleV(potion.position, potion.radius, potion.color);
            DrawBullets(bullets);
            DrawText(TextFormat("Active Bullets: %d", CountActiveBullets(bullets)), 10, 10, 20, BLACK);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
