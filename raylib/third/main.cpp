#include "raylib.h"

#include "raymath.h"

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------

// Movement constants
constexpr float GRAVITY{ 32.0f };
constexpr float MAX_SPEED{ 20.0f };
constexpr float CROUCH_SPEED{ 5.0f };
constexpr float JUMP_FORCE{ 12.0f };
constexpr float MAX_ACCEL{ 150.0f };

// Grounded drag
constexpr float FRICTION{ 0.86f };

// Increasing air drag, increases strafing speed
constexpr float AIR_DRAG{ 0.98f };

// Responsiveness for turning movement direction to looked direction
constexpr float CONTROL{ 15.0f };
constexpr float CROUCH_HEIGHT{ 0.0f };
constexpr float STAND_HEIGHT{ 1.0f };
constexpr float BOTTOM_HEIGHT{ 0.5f };

#define NORMALIZE_INPUT  0

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
// Body structure
struct Body {
    Vector3 position{};
    Vector3 velocity{};
    Vector3 dir{};
    bool isGrounded{};
};

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------

namespace {
    Vector2 sensitivity { 0.001f, 0.001f };
    
    Body player {};
    Vector2 lookRotation {};
    float headTimer {};
    float walkLerp {};
    float headLerp { STAND_HEIGHT };
    Vector2 lean {};
}

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
static void DrawLevel(void);
static void UpdateCameraFPS(Camera& camera);
static void UpdateBody(Body& body, float rot, int side, int forward, bool jumpPressed, bool crouchHold);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    constexpr int screenWidth { 800 };
    constexpr int screenHeight { 450 };

    InitWindow(screenWidth, screenHeight, "raylib [core] example - 3d camera fps");

    // Initialize camera variables
    // NOTE: UpdateCameraFPS() takes care of the rest
    Camera camera {};
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    camera.position = {
        player.position.x,
        player.position.y + (BOTTOM_HEIGHT + headLerp),
        player.position.z,
    };

    UpdateCameraFPS(camera); // Update camera parameters

    DisableCursor();        // Limit cursor to relative movement inside the window

    SetTargetFPS(60);       // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        const Vector2 mouseDelta { GetMouseDelta() };
        lookRotation.x -= mouseDelta.x*sensitivity.x;
        lookRotation.y += mouseDelta.y*sensitivity.y;

        int sideway { IsKeyDown(KEY_D) - IsKeyDown(KEY_A) };
        int forward { IsKeyDown(KEY_W) - IsKeyDown(KEY_S) };
        bool crouching { IsKeyDown(KEY_LEFT_CONTROL) };
        UpdateBody(player, lookRotation.x, sideway, forward, IsKeyPressed(KEY_SPACE), crouching);

        const float delta { GetFrameTime() };
        headLerp = Lerp(headLerp, crouching ? CROUCH_HEIGHT : STAND_HEIGHT, 20.0f*delta);
        camera.position = {
            player.position.x,
            player.position.y + (BOTTOM_HEIGHT + headLerp),
            player.position.z,
        };

        if (player.isGrounded && (forward != 0 || sideway != 0))
        {
            headTimer += delta*3.0f;
            walkLerp = Lerp(walkLerp, 1.0f, 10.0f*delta);
            camera.fovy = Lerp(camera.fovy, 55.0f, 5.0f*delta);
        }
        else
        {
            walkLerp = Lerp(walkLerp, 0.0f, 10.0f*delta);
            camera.fovy = Lerp(camera.fovy, 60.0f, 5.0f*delta);
        }

        lean.x = Lerp(lean.x, sideway*0.02f, 10.0f*delta);
        lean.y = Lerp(lean.y, forward*0.015f, 10.0f*delta);

        UpdateCameraFPS(camera);
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);
                DrawLevel();
            EndMode3D();

            // Draw info box
            DrawRectangle(5, 5, 330, 75, Fade(SKYBLUE, 0.5f));
            DrawRectangleLines(5, 5, 330, 75, BLUE);

            DrawText("Camera controls:", 15, 15, 10, BLACK);
            DrawText("- Move keys: W, A, S, D, Space, Left-Ctrl", 15, 30, 10, BLACK);
            DrawText("- Look around: arrow keys or mouse", 15, 45, 10, BLACK);
            DrawText(TextFormat("- Velocity Len: (%06.3f)", Vector2Length(Vector2{ player.velocity.x, player.velocity.z })), 15, 60, 10, BLACK);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------
// Update body considering current world state
void UpdateBody(Body& body, float rot, int side, int forward, bool jumpPressed, bool crouchHold)
{
    Vector2 input { static_cast<float>(side), static_cast<float>(-forward) };

#if defined(NORMALIZE_INPUT)
    // Slow down diagonal movement
    if (side != 0 && forward != 0) input = Vector2Normalize(input);
#endif

    const float delta { GetFrameTime() };

    if (!body.isGrounded) body.velocity.y -= GRAVITY*delta;

    if (body.isGrounded && jumpPressed)
    {
        body.velocity.y = JUMP_FORCE;
        body.isGrounded = false;

        // Sound can be played at this moment
        //SetSoundPitch(fxJump, 1.0f + (GetRandomValue(-100, 100)*0.001));
        //PlaySound(fxJump);
    }

    const Vector3 front { sinf(rot), 0.f, cosf(rot) };
    const Vector3 right { cosf(-rot), 0.f, sinf(-rot) };

    const Vector3 desiredDir { input.x*right.x + input.y*front.x, 0.0f, input.x*right.z + input.y*front.z, };
    body.dir = Vector3Lerp(body.dir, desiredDir, CONTROL*delta);

    const float decel { body.isGrounded ? FRICTION : AIR_DRAG };
    Vector3 hvel { body.velocity.x*decel, 0.0f, body.velocity.z*decel };

    const float hvelLength { Vector3Length(hvel) }; // Magnitude
    if (hvelLength < MAX_SPEED*0.01f) hvel = Vector3{ 0 };

    // This is what creates strafing
    const float speed { Vector3DotProduct(hvel, body.dir) };

    // Whenever the amount of acceleration to add is clamped by the maximum acceleration constant,
    // a Player can make the speed faster by bringing the direction closer to horizontal velocity angle
    // More info here: https://youtu.be/v3zT3Z5apaM?t=165
    const float maxSpeed { crouchHold? CROUCH_SPEED : MAX_SPEED };
    const float accel { Clamp(maxSpeed - speed, 0.f, MAX_ACCEL*delta) };
    hvel.x += body.dir.x*accel;
    hvel.z += body.dir.z*accel;

    body.velocity.x = hvel.x;
    body.velocity.z = hvel.z;

    body.position.x += body.velocity.x*delta;
    body.position.y += body.velocity.y*delta;
    body.position.z += body.velocity.z*delta;

    // Fancy collision system against the floor
    if (body.position.y <= 0.0f)
    {
        body.position.y = 0.0f;
        body.velocity.y = 0.0f;
        body.isGrounded = true; // Enable jumping
    }
}

// Update camera for FPS behaviour
static void UpdateCameraFPS(Camera& camera)
{
    constexpr Vector3 up { 0.0f, 1.0f, 0.0f };
    constexpr Vector3 targetOffset { 0.0f, 0.0f, -1.0f };

    // Left and right
    Vector3 yaw { Vector3RotateByAxisAngle(targetOffset, up, lookRotation.x) };

    // Clamp view up
    float maxAngleUp { Vector3Angle(up, yaw) };
    maxAngleUp -= 0.001f; // Avoid numerical errors
    if ( -(lookRotation.y) > maxAngleUp) { lookRotation.y = -maxAngleUp; }

    // Clamp view down
    float maxAngleDown { Vector3Angle(Vector3Negate(up), yaw) };
    maxAngleDown *= -1.0f; // Downwards angle is negative
    maxAngleDown += 0.001f; // Avoid numerical errors
    if ( -(lookRotation.y) < maxAngleDown) { lookRotation.y = -maxAngleDown; }

    // Up and down
    Vector3 right { Vector3Normalize(Vector3CrossProduct(yaw, up)) };

    // Rotate view vector around right axis
    float pitchAngle { -lookRotation.y - lean.y };
    pitchAngle = Clamp(pitchAngle, -PI/2 + 0.0001f, PI/2 - 0.0001f); // Clamp angle so it doesn't go past straight up or straight down
    Vector3 pitch { Vector3RotateByAxisAngle(yaw, right, pitchAngle) };

    // Head animation
    // Rotate up direction around forward axis
    const float headSin { sinf(headTimer*PI) };
    const float headCos { cosf(headTimer*PI) };
    constexpr float stepRotation { 0.01f };
    camera.up = Vector3RotateByAxisAngle(up, pitch, headSin*stepRotation + lean.x);

    // Camera BOB
    constexpr float bobSide { 0.1f };
    constexpr float bobUp { 0.15f };
    Vector3 bobbing { Vector3Scale(right, headSin*bobSide) };
    bobbing.y = std::abs(headCos*bobUp);

    camera.position = Vector3Add(camera.position, Vector3Scale(bobbing, walkLerp));
    camera.target = Vector3Add(camera.position, pitch);
}

// Draw game level
static void DrawLevel(void)
{
    constexpr int floorExtent { 25 };
    constexpr float tileSize { 5.0f };
    constexpr Color tileColor1 { 150, 200, 200, 255 };

    // Floor tiles
    for (int y = -floorExtent; y < floorExtent; y++)
    {
        for (int x = -floorExtent; x < floorExtent; x++)
        {
            if ((y & 1) && (x & 1))
            {
                DrawPlane(Vector3{ x*tileSize, 0.0f, y*tileSize}, Vector2{ tileSize, tileSize }, tileColor1);
            }
            else if (!(y & 1) && !(x & 1))
            {
                DrawPlane(Vector3{ x*tileSize, 0.0f, y*tileSize}, Vector2{ tileSize, tileSize }, LIGHTGRAY);
            }
        }
    }

    constexpr Vector3 towerSize { 16.0f, 32.0f, 16.0f };
    constexpr Color towerColor { 150, 200, 200, 255 };

    Vector3 towerPos { 16.0f, 16.0f, 16.0f };
    DrawCubeV(towerPos, towerSize, towerColor);
    DrawCubeWiresV(towerPos, towerSize, DARKBLUE);

    towerPos.x *= -1;
    DrawCubeV(towerPos, towerSize, towerColor);
    DrawCubeWiresV(towerPos, towerSize, DARKBLUE);

    towerPos.z *= -1;
    DrawCubeV(towerPos, towerSize, towerColor);
    DrawCubeWiresV(towerPos, towerSize, DARKBLUE);

    towerPos.x *= -1;
    DrawCubeV(towerPos, towerSize, towerColor);
    DrawCubeWiresV(towerPos, towerSize, DARKBLUE);

    towerPos = Vector3{ 48.0f, 16.0f, 16.0f };
    DrawCubeV(towerPos, towerSize, towerColor);
    DrawCubeWiresV(towerPos, towerSize, DARKBLUE);

    towerPos = Vector3{ 80.0f, 16.0f, 16.0f };
    DrawCubeV(towerPos, towerSize, towerColor);
    DrawCubeWiresV(towerPos, towerSize, DARKBLUE);
    // Red sun
    DrawSphere(Vector3{ 300.0f, 300.0f, 0.0f }, 100.0f, Color{ 255, 0, 0, 255 });
}
