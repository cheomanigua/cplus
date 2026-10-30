#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <float.h>
#include <math.h>

typedef struct { uint32_t pointCount; Vector2 *points; } Coastline;
typedef struct { uint32_t coastlineCount; Coastline *coastlines; Vector2 min, max; } Map;

static void UnloadMap(Map *map)
{
    if (!map) return;
    for (uint32_t i = 0; i < map->coastlineCount; i++) free(map->coastlines[i].points);
    free(map->coastlines);
    map->coastlines = NULL;
    map->coastlineCount = 0;
}

static Map LoadMap(const char *filename)
{
    Map map = {0};
    map.min = (Vector2){FLT_MAX, FLT_MAX};
    map.max = (Vector2){-FLT_MAX, -FLT_MAX};

    FILE *file = fopen(filename, "rb");
    if (!file) { printf("ERROR: Could not open %s\n", filename); return map; }

    char magic[4];
    uint32_t version;
    if (fread(magic, 1, 4, file) != 4 ||
        magic[0] != 'M' || magic[1] != 'A' || magic[2] != 'P' || magic[3] != '2' ||
        fread(&version, sizeof(version), 1, file) != 1 || version != 1 ||
        fread(&map.coastlineCount, sizeof(map.coastlineCount), 1, file) != 1)
    {
        printf("ERROR: Invalid map header.\n");
        fclose(file);
        map.coastlineCount = 0;
        return map;
    }

    map.coastlines = calloc(map.coastlineCount, sizeof(Coastline));
    if (!map.coastlines) { fclose(file); map.coastlineCount = 0; return map; }

    for (uint32_t i = 0; i < map.coastlineCount; i++)
    {
        uint32_t n;
        if (fread(&n, sizeof(n), 1, file) != 1) { fclose(file); UnloadMap(&map); return (Map){0}; }

        map.coastlines[i].pointCount = n;
        map.coastlines[i].points = malloc(sizeof(Vector2) * n);
        if (!map.coastlines[i].points) { fclose(file); UnloadMap(&map); return (Map){0}; }

        for (uint32_t j = 0; j < n; j++)
        {
            float x, y;
            if (fread(&x, sizeof(float), 1, file) != 1 ||
                fread(&y, sizeof(float), 1, file) != 1)
            {
                fclose(file);
                UnloadMap(&map);
                return (Map){0};
            }

            map.coastlines[i].points[j] = (Vector2){x, y};
            if (x < map.min.x) map.min.x = x;
            if (x > map.max.x) map.max.x = x;
            if (y < map.min.y) map.min.y = y;
            if (y > map.max.y) map.max.y = y;
        }
    }

    fclose(file);

    printf("Map loaded: %u coastline segments\n", map.coastlineCount);
    printf("Size: %.1f x %.1f km\n",
           (map.max.x - map.min.x) / 1000.0f,
           (map.max.y - map.min.y) / 1000.0f);
    return map;
}

static Vector2 WorldToScreen(Vector2 world, Vector2 target, Vector2 offset, float zoom)
{
    return (Vector2){
        offset.x + (world.x - target.x) * zoom,
        offset.y - (world.y - target.y) * zoom
    };
}

static Vector2 ScreenToWorld(Vector2 screen, Vector2 target, Vector2 offset, float zoom)
{
    return (Vector2){
        target.x + (screen.x - offset.x) / zoom,
        target.y - (screen.y - offset.y) / zoom
    };
}

static void DrawMap(const Map *map, Vector2 target, Vector2 offset, float zoom)
{
    for (uint32_t i = 0; i < map->coastlineCount; i++)
    {
        const Coastline *line = &map->coastlines[i];
        for (uint32_t j = 1; j < line->pointCount; j++)
        {
            Vector2 a = WorldToScreen(line->points[j - 1], target, offset, zoom);
            Vector2 b = WorldToScreen(line->points[j], target, offset, zoom);
            DrawLineV(a, b, WHITE);
        }
    }
}

static void DrawWorldGrid(Vector2 target, Vector2 offset, float zoom, int width, int height)
{
    float pxPerKm = zoom * 1000.0f;
    float spacing;

    if (pxPerKm > 300) spacing = 100;
    else if (pxPerKm > 150) spacing = 250;
    else if (pxPerKm > 75) spacing = 500;
    else if (pxPerKm > 30) spacing = 1000;
    else if (pxPerKm > 10) spacing = 5000;
    else if (pxPerKm > 3) spacing = 10000;
    else if (pxPerKm > 1) spacing = 50000;
    else spacing = 100000;

    Vector2 tl = ScreenToWorld((Vector2){0, 0}, target, offset, zoom);
    Vector2 br = ScreenToWorld((Vector2){(float)width, (float)height}, target, offset, zoom);

    float minX = floorf(tl.x / spacing) * spacing;
    float maxX = ceilf(br.x / spacing) * spacing;
    float minY = floorf(br.y / spacing) * spacing;
    float maxY = ceilf(tl.y / spacing) * spacing;

    Color grid = {60, 75, 90, 80};

    for (float x = minX; x <= maxX; x += spacing)
        DrawLineV(WorldToScreen((Vector2){x, minY}, target, offset, zoom),
                  WorldToScreen((Vector2){x, maxY}, target, offset, zoom), grid);

    for (float y = minY; y <= maxY; y += spacing)
        DrawLineV(WorldToScreen((Vector2){minX, y}, target, offset, zoom),
                  WorldToScreen((Vector2){maxX, y}, target, offset, zoom), grid);
}

static void DrawScaleBar(float zoom, int screenHeight)
{
    const float targetPixels = 150.0f;
    float wanted = targetPixels / zoom;
    if (wanted <= 0) return;

    float magnitude = powf(10.0f, floorf(log10f(wanted)));
    float n = wanted / magnitude;
    float distance = (n <= 1) ? magnitude : (n <= 2) ? 2*magnitude :
                     (n <= 5) ? 5*magnitude : 10*magnitude;

    float barPixels = distance * zoom;
    if (barPixels > 220) { distance *= 0.5f; barPixels = distance * zoom; }
    if (barPixels < 60)  { distance *= 2.0f; barPixels = distance * zoom; }

    float x = 25.0f, y = (float)screenHeight - 45.0f;

    DrawRectangle((int)(x - 10), (int)(y - 30),
                  (int)(barPixels + 20), 55, (Color){0,0,0,170});

    DrawLineEx((Vector2){x,y}, (Vector2){x+barPixels,y}, 4, WHITE);
    DrawLineEx((Vector2){x,y-7}, (Vector2){x,y+7}, 3, WHITE);
    DrawLineEx((Vector2){x+barPixels,y-7}, (Vector2){x+barPixels,y+7}, 3, WHITE);

    char label[64];
    if (distance < 1000)
        snprintf(label, sizeof(label), "%.0f m", distance);
    else
    {
        float km = distance / 1000.0f;
        if (km >= 10)
            snprintf(label, sizeof(label), "%.0f km", km);
        else
            snprintf(label, sizeof(label), "%.1f km", km);
    }

    int tw = MeasureText(label, 18);
    DrawText(label, (int)(x + barPixels/2 - tw/2), (int)(y - 27), 18, WHITE);
}

int main(void)
{
    const int screenWidth = 1280;
    const int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "Harpoon Map");
    SetTargetFPS(60);

    Map map = LoadMap("east_asia.map");
    if (!map.coastlines) { CloseWindow(); return 1; }

    Vector2 center = {
        (map.min.x + map.max.x) * 0.5f,
        (map.min.y + map.max.y) * 0.5f
    };

    float mapWidth = map.max.x - map.min.x;
    float mapHeight = map.max.y - map.min.y;

    Vector2 target = center;
    Vector2 offset = {screenWidth/2.0f, screenHeight/2.0f};

    float zoomX = screenWidth * 0.90f / mapWidth;
    float zoomY = screenHeight * 0.90f / mapHeight;
    float zoom = (zoomX < zoomY) ? zoomX : zoomY;

    Vector2 shipPosition = center;
    const float shipLength = 155.0f;
    const float shipWidth = 20.0f;

    while (!WindowShouldClose())
    {
        float wheel = GetMouseWheelMove();

        if (wheel != 0)
        {
            Vector2 before = ScreenToWorld(GetMousePosition(), target, offset, zoom);

            if (wheel > 0) zoom *= 1.15f;
            else zoom /= 1.15f;

            if (zoom < 0.000001f) zoom = 0.000001f;
            if (zoom > 10.0f) zoom = 10.0f;

            Vector2 after = ScreenToWorld(GetMousePosition(), target, offset, zoom);
            target.x += before.x - after.x;
            target.y += before.y - after.y;
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE))
        {
            Vector2 d = GetMouseDelta();
            target.x -= d.x / zoom;
            target.y += d.y / zoom;
        }

        BeginDrawing();
        ClearBackground((Color){15,35,55,255});

        DrawWorldGrid(target, offset, zoom, screenWidth, screenHeight);
        DrawMap(&map, target, offset, zoom);

        Vector2 ship = WorldToScreen(shipPosition, target, offset, zoom);
        float sw = shipLength * zoom;
        float sh = shipWidth * zoom;

        DrawRectangleRec(
            (Rectangle){ship.x-sw/2, ship.y-sh/2, sw, sh},
            RED);

        DrawRectangle(10,10,310,180,(Color){0,0,0,160});
        DrawText("HARPOON MAP",20,20,30,WHITE);
        DrawText("Middle mouse: pan",20,55,20,WHITE);
        DrawText("Mouse wheel: zoom",20,80,20,WHITE);
        DrawText(TextFormat("Zoom: %.6f px/m",zoom),20,105,20,WHITE);
        DrawText(TextFormat("Map: %.0f x %.0f km",
                            mapWidth/1000.0f,mapHeight/1000.0f),20,130,20,WHITE);
        DrawText("Ship: 155m x 20m",20,155,20,WHITE);

        DrawScaleBar(zoom, screenHeight);

        DrawLine(screenWidth-60,80,screenWidth-60,40,WHITE);
        DrawTriangle(
            (Vector2){screenWidth-60,30},
            (Vector2){screenWidth-68,45},
            (Vector2){screenWidth-52,45},
            WHITE);
        DrawText("N",screenWidth-67,85,20,WHITE);

        EndDrawing();
    }

    UnloadMap(&map);
    CloseWindow();
    return 0;
}
