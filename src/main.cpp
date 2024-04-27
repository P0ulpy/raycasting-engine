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

    float Yaw()
    {
        return Vector2DirectionToAngle(direction);
    }

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

    Vector2 Right()
    {
        Vector2 forward = Forward();
        return {
            -forward.y, forward.x
        };
    }

    void LookAt(float x, float y) { LookAt({x, y}); }
    void LookAt(const Vector2& positionLook)
    {
        Vector2 direction = Vector2Subtract(positionLook, position);
        direction = Vector2Normalize(direction);

        yaw = Vector2DirectionToAngle(direction);
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

    bool moveMode   = true;
    bool view3DMode = false;

    while (!WindowShouldClose())
    {
        float currentTime = GetTime();
        deltaTime = currentTime - lastFrameTime;
        lastFrameTime = currentTime;

        // Inputs

        if(IsKeyPressed(KEY_TAB))
        {
            view3DMode = !view3DMode;
        }

        if(IsKeyPressed(KEY_LEFT_CONTROL) || IsKeyPressed(KEY_RIGHT_CONTROL))
        {
            moveMode = !moveMode;
        }

        if(moveMode)
        {
            if(!view3DMode)
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
            else
            {
                {
                    Vector2 moveDirection {0};

                    if(IsKeyDown(KEY_D))
                    {
                        moveDirection = Vector2Add(moveDirection, cam.Right());
                    }
                    if(IsKeyDown(KEY_A))
                    {
                        Vector2 left = Vector2Negate(cam.Right());
                        moveDirection = Vector2Add(moveDirection, left);
                    }
                    if(IsKeyDown(KEY_W))
                    {
                        moveDirection = Vector2Add(moveDirection, cam.Forward());
                    }
                    if(IsKeyDown(KEY_S))
                    {
                        Vector2 backward = Vector2Negate(cam.Forward());
                        moveDirection = Vector2Add(moveDirection, backward);
                    }

                    float moveSpeed = 100.f;

                    moveDirection = Vector2Normalize(moveDirection);
                    moveDirection = Vector2Scale(moveDirection, moveSpeed);
                    moveDirection = Vector2Scale(moveDirection, deltaTime);

                    cam.position = Vector2Add(cam.position, moveDirection);
                }

                {
                    // Get the mouse delta (how much the mouse moved since the last frame)
                    Vector2 mouseDelta = GetMouseDelta();
                    SetMousePosition(GetScreenWidth() / 2, GetScreenHeight() / 2);

                    // Scale the mouse movement by a sensitivity factor to control the speed of the rotation
                    constexpr float sensitivity = 0.005f;
                    float yawChange = mouseDelta.x * sensitivity;

                    // Update the camera's yaw
                    cam.yaw += yawChange;

                    // Make sure the yaw is between 0 and 2 * PI
                    if(cam.yaw > 2 * PI) cam.yaw -= 2 * PI; 
                    if(cam.yaw < 0)      cam.yaw += 2 * PI;
                }
            }
        }

        // Update
        

        // Draw

        BeginDrawing();

        if(view3DMode && moveMode)
        {
            HideCursor();
        }
        else
        {
            ShowCursor();
        }

        if(view3DMode)
            ClearBackground(BLACK);
        else
            ClearBackground(LIGHTGRAY);


        {
            constexpr float viewMaxDistance = 1000.0f;

            RasterRay ray(cam.position);

            float fovRate = cam.fov / GetScreenWidth();
            float angle = -(cam.fov / 2);

            for(int screenX = 0; screenX < GetScreenWidth(); ++screenX)
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
                    if(!view3DMode)
                    {
                        DrawLineV(cam.position, hitPosition, BLUE);
                        DrawCircleV(hitPosition, 1, RED);                        
                    }
                    else
                    {
                        // Normalize distance to [0, 1]
                        float tBrightness = Clamp(hitDistance, 0, viewMaxDistance) / viewMaxDistance;
                        uint8_t brightness = Lerp(255, 0, tBrightness);
                        
                        constexpr float ProjectionDistance = 100.f;

                        float forwardHeadingAngle = ray.Yaw() - cam.yaw;
                        float headingLenght = hitDistance * cosf(forwardHeadingAngle);

                        float rayDirectionDeg = cam.fov * (floor(0.5 * GetScreenWidth()) - screenX) / GetScreenWidth();
                        float rayProjectionPosition = 0.5 * tanf(rayDirectionDeg * DEG2RAD) / tanf((0.5 * cam.fov) * DEG2RAD);

                        float height = GetScreenHeight() * ProjectionDistance / (hitDistance * cosf(rayDirectionDeg * DEG2RAD));
                        
                        // float height = 200.f;
 
                        {
                            float heightDelta = GetScreenHeight() - height;
                            float topY = (heightDelta > 0) ? heightDelta / 2 : heightDelta;
                            float bottomY = topY + height;

                            DrawLineV(
                                { (float)screenX, topY }, 
                                { (float)screenX, bottomY }, 
                                { 166, 46, 90, brightness }
                            );
                        }
                    }
                }
            }
        }

        if(!view3DMode)
        {
            // Cam forward line
            Vector2 camHeadingDirectionPoint = Vector2Add(cam.position, Vector2Scale(cam.Forward(), 50));
            DrawLineV(cam.position, camHeadingDirectionPoint, RED);
            DrawCircleV(cam.position, 10, GREEN);

            for(size_t i = 0; i < WorldSize; i++)
            {
                World[i].Draw();
            }
        }

        // Draw GUI

        constexpr auto view3DTextEnable = "[TAB] 3DView enabled";
        constexpr auto view3DTextDisable = "[TAB] 3DView disabled";

        DrawText((view3DMode) ? view3DTextEnable : view3DTextDisable, 20, 20, 20, (view3DMode) ? GREEN : RED);

        constexpr auto MoveModeTextEnable = "[CTRL] Move mode enabled";
        constexpr auto MoveModeTextDisable = "[CTRL] Move mode disabled";

        DrawText((moveMode) ? MoveModeTextEnable : MoveModeTextDisable, 20, 40, 20, (moveMode) ? GREEN : RED);

        // Draw ImGUI

        rlImGuiBegin();
            cam.DrawGUI();
        rlImGuiEnd();

        EndDrawing();
    }

    rlImGuiShutdown();
    CloseWindow();

    return 0;
}

