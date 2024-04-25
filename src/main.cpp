#include <raylib.h>
#define RAYGUI_IMPLEMENTATION

#include <raymath.h>

#include <raygui.h>
#include <rlImGui.h>

#include <imgui.h>
#include <imgui_stdlib.h>

#include <cmath>
#include <vector>
#include <limits>

constexpr int DefaultScreenWidth = 1720;
constexpr int DefaultScreenHeight = 880;

Vector2 Vector2DirectionFromAngle(float angleRadian, float length = 1)
{
    return {
        length * cosf(angleRadian),
        length * sinf(angleRadian),
    };
}

float Vector2DirectionToAngle(Vector2 direction)
{
    return atan2f(direction.y, direction.x);
}

struct Segment
{
    Vector2 a { 0 };
    Vector2 b { 0 };

    Segment(Vector2 a, Vector2 b) : a(a), b(b) {}

    void Draw()
    {
        DrawLineV(a, b, GRAY);
    }
};

struct RasterRay
{
    Vector2 position { 0 };
    Vector2 direction { 0 };

    RasterRay() = default;
    RasterRay(Vector2 pos) : position(pos) {}
    RasterRay(Vector2 pos, Vector2 dir) : position(pos), direction(dir) {}

    void LookAt(const Vector2& positionLook)
    {
        direction = Vector2Subtract(position, positionLook);
        direction = Vector2Normalize(direction);
    }
};

struct HitInfo
{
    Vector2 position { 0 };
};

bool RayToSegmentCollision(const RasterRay& ray, const Segment& seg, HitInfo& hitInfo)
{
    const float x1 = seg.a.x;
    const float y1 = seg.a.y;
    const float x2 = seg.b.x;
    const float y2 = seg.b.y;
    
    const float x3 = ray.position.x;
    const float y3 = ray.position.y;
    const float x4 = ray.position.x + ray.direction.x;
    const float y4 = ray.position.y + ray.direction.y;

    const float devider = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);

    // the tow segements are perfectly parallels
    if(devider == 0) return false;

    const float t = ((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)) / devider;
    const float u = ((x1 - x3) * (y1 - y2) - (y1 - y3) * (x1 - x2)) / devider;
    
    // old wikipedia formula for u
    //const u = -((x1 - x2) * (y1 - y3) - (y1 - y2) * (x1 - x3)) / devider;
    
    if(t > 0 && t < 1 && u > 0)
    {
        const float xCollision = x1 + t * (x2 - x1);
        const float yCollision = y1 + t * (y2 - y1);

        const Vector2 hitPosition = { xCollision, yCollision };
        
        hitInfo  = {
            .position = hitPosition
        };

        return true;
    }

    return false;
}

struct RaycasterCamera
{
    Vector2 position { DefaultScreenWidth / 2, DefaultScreenHeight / 2 };
    float yaw { 0 };
    // float pitch; // later ;)

    float fov { 60 };

    Vector2 Forward()
    {
        return Vector2DirectionFromAngle(yaw);
    }

    void LookAt(float x, float y) { LookAt({x, y}); }
    void LookAt(const Vector2& positionLook)
    {
        Vector2 direction = Vector2Subtract(positionLook, position);
        direction = Vector2Normalize(direction);

        yaw = Vector2DirectionToAngle(direction);
    }
    
    void Draw()
    {
        DrawCircleV(position, 10, GREEN);
    }

    void DrawGUI()
    {
        ImGui::Begin("Camera");
        
        ImGui::InputFloat2("position", (float*)(&position));
        ImGui::SliderAngle("yaw", &yaw);

        ImGui::SliderFloat("FOV", &fov, 20, 180);

        ImGui::End();
    }
};

Segment World[] =
{
    { { 500, 200 }, { 500, 500 } },
    { { 500, 500 }, { 200, 550 } },
};

constexpr size_t WorldSize = sizeof(World) / sizeof(Segment);

int main()
{
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(DefaultScreenWidth, DefaultScreenHeight, "wall-it");
    rlImGuiSetup(true);

    SetTargetFPS(144);

    float deltaTime = 0;
    float lastFrameTime = GetTime();

    RaycasterCamera cam;

    bool moveMode = false;

    while (!WindowShouldClose())
    {
        float currentTime = GetTime();
        deltaTime = currentTime - lastFrameTime;
        lastFrameTime = currentTime;

        // Inputs

        moveMode = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);        

        if(moveMode)
        {
            if(IsMouseButtonDown(MOUSE_BUTTON_LEFT))
            {
                cam.position.x = GetMouseX();
                cam.position.y = GetMouseY();
            }
            if(IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
            {
                cam.LookAt((float)GetMouseX(), (float)GetMouseY());
            }
        }

        // Update
        

        // Draw

        BeginDrawing();

        ClearBackground(LIGHTGRAY);

        RasterRay ray(cam.position);

        float fovRate = cam.fov / GetScreenWidth();
        float angle = -(cam.fov / 2);

        for(int w = 0; w < GetScreenWidth(); w++)
        {
            angle += fovRate;
            ray.direction = Vector2DirectionFromAngle((angle * DEG2RAD) + cam.yaw);

            float hitDistance = std::numeric_limits<float>::max();
            Vector2 hitPosition;

            for(size_t i = 0; i < WorldSize; i++)
            {
                HitInfo hit;

                if(RayToSegmentCollision(ray, World[i], hit))
                {
                    float distance = Vector2Distance(ray.position, hit.position);

                    if(distance < hitDistance)
                    {
                        hitDistance = distance;
                        hitPosition = hit.position;
                    }
                }
            }

            if(hitDistance < std::numeric_limits<float>::max())
            {
                DrawLineV(cam.position, hitPosition, BLUE);
                DrawCircleV(hitPosition, 1, RED);
            }
        }

        // Cam forward line
        Vector2 camHeadingDirectionPoint = Vector2Add(cam.position, Vector2Scale(cam.Forward(), 50));
        DrawLineV(cam.position, camHeadingDirectionPoint, RED);

        for(size_t i = 0; i < WorldSize; i++)
        {
            World[i].Draw();
        }

        cam.Draw();

        // Draw GUI

        constexpr auto MoveModeTextEnable = "Hold [CTRL] Move mode enabled";
        constexpr auto MoveModeTextDisable = "Hold [CTRL] Move mode disabled";

        DrawText((moveMode) ? MoveModeTextEnable : MoveModeTextDisable, 20, 20, 20, (moveMode) ? GREEN : RED);

        rlImGuiBegin();
            cam.DrawGUI();
        rlImGuiEnd();

        EndDrawing();
    }

    rlImGuiShutdown();
    CloseWindow();

    return 0;
}

