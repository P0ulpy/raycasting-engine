#include <raylib.h>
#define RAYGUI_IMPLEMENTATION

#include <raymath.h>

#include <raygui.h>
#include <rlImGui.h>

#include <imgui.h>
#include <imgui_stdlib.h>

#include <cmath>

constexpr int DefaultScreenWidth = 1000;
constexpr int DefaultScreenHeight = 650;


Vector2 Vector2FromAngle(float angleRadian, float length = 1)
{
    return {
        length * cosf(angleRadian),
        length * sinf(angleRadian),
    };
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

    RasterRay(Vector2 pos, Vector2 dir) : position(pos), direction(dir) {}

    void LookAt(const Vector2& positionLook)
    {
        direction = Vector2Subtract(position, positionLook);
        direction = Vector2Normalize(direction);
    }
};

struct HitInfo
{
    Vector2 hitPosition;
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
            .hitPosition = hitPosition
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

    float fov { 90 };

    Vector2 Forward()
    {
        return Vector2FromAngle(yaw);
    }

    // void LookAt(const Vector2& positionLook)
    // {
    //     direction = Vector2Subtract(position, positionLook);
    //     direction = Vector2Normalize(direction);
    // }
    
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

    float deltaTime = 0.0f;
    float lastFrameTime = GetTime();

    RaycasterCamera cam;

    while (!WindowShouldClose())
    {
        float currentTime = GetTime();
        deltaTime = currentTime - lastFrameTime;
        lastFrameTime = currentTime;

        // Update

        if(IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
        {
            cam.position.x = GetMouseX();
            cam.position.y = GetMouseY();
        }
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            // cam.LookAt({ GetMouseX(), GetMouseY()});
        }

        BeginDrawing();

        ClearBackground(LIGHTGRAY);

        // Draw

        RasterRay ray(cam.position, cam.Forward());

        HitInfo hit;

        for(size_t i = 0; i < WorldSize; i++)
        {
            if(RayToSegmentCollision(ray, World[i], hit))
            {
                DrawLineV(cam.position, hit.hitPosition, BLUE);
                DrawCircleV(hit.hitPosition, 5, BLUE);
            }
        }

        for(size_t i = 0; i < WorldSize; i++)
        {
            World[i].Draw();
        }

        cam.Draw();

        rlImGuiBegin();
            cam.DrawGUI();
        rlImGuiEnd();

        EndDrawing();
    }

    rlImGuiShutdown();
    CloseWindow();

    return 0;
}

