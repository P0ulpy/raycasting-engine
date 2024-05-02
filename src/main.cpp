#include <raylib.h>
#include <raymath.h>
#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

#include <rlImGui.h>
#include <imgui.h>
#include <imgui_stdlib.h>

#include <cmath>
#include <vector>
#include <limits>
#include <sstream>

constexpr int DefaultScreenWidth = 1720;
constexpr int DefaultScreenHeight = 880;

constexpr Color MY_RED          { 166, 46, 90, 255 };
constexpr Color MY_PURPLE       { 89, 27, 79, 255 };
constexpr Color MY_DARK_BLUE    { 40, 17, 64, 255 };
constexpr Color MY_BEIGE        { 242, 211, 172, 255 };
constexpr Color MY_BLACK        { 13, 13, 13, 255 };

constexpr Color ColorAlpha255(Color color, uint8_t alpha)
{
    return { color.r, color.g, color.b, alpha };
}


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
    float yaw   { 0 };
    float pitch { 0 };

    float fov { 60 };
    float fovVectical { 120 };

    float farPlaneDistance = 1000.0f;
    float nearPlaneDistance = 100.f;

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
        ImGui::SliderFloat("yaw", &yaw, -90, 90);
        ImGui::SliderAngle("pitch", &pitch);
        
        ImGui::SliderFloat("FOV", &fov, 20, 180);
        ImGui::SliderFloat("FOV Vetical", &fovVectical, 20, 180);
        
        ImGui::SliderFloat("Far plane Distance", &farPlaneDistance, 1, 5000);
        ImGui::SliderFloat("Near plane Distance", &nearPlaneDistance, 1, 500);

        ImGui::End();
    }
};

const Segment World[] =
{
    { { 500, 200 }, { 500, 500 } },
    { { 500, 500 }, { 200, 550 } },
};

constexpr size_t WorldSize = sizeof(World) / sizeof(Segment);

int main()
{
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(DefaultScreenWidth, DefaultScreenHeight, "raycasting-engine");
    SetExitKey(KEY_NULL);

    rlImGuiSetup(true);
    ImGuiIO& imguiIO = ImGui::GetIO();
    imguiIO.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; 
    imguiIO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    imguiIO.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; 
    
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

        std::string windowTitle = "raycasting-engine [";
        windowTitle += std::to_string(1.0f / deltaTime);
        windowTitle += " FPS]";

        SetWindowTitle(windowTitle.c_str());

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
                    constexpr float sensitivity = 0.003f;
                    float yawChange = mouseDelta.x * sensitivity;
                    float pitchChange = mouseDelta.y * sensitivity;

                    // Update the camera's yaw
                    cam.yaw += yawChange;
                    cam.pitch += pitchChange;

                    // Make sure the yaw is between 0 and 2 * PI
                    if(cam.yaw > 2 * PI) cam.yaw -= 2 * PI; 
                    if(cam.yaw < 0)      cam.yaw += 2 * PI;

                    // Make sure the pitcj is between -(PI / 2) and PI / 2
                    if(cam.pitch > PI / 2)  cam.pitch = PI / 2;
                    if(cam.pitch < -PI / 2) cam.pitch = -PI / 2;
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
        {            
            ClearBackground(MY_BLACK);
        }
        else
        {
            ClearBackground(LIGHTGRAY);
        }

        // Raycasting rendering
        {
            float floorVerticalOffset = round(0.5f * GetScreenHeight() * (tanf(cam.pitch)) / tanf(0.5f * cam.fovVectical));

            // floor / ceiling rendering
            // {
            //     float center = (GetScreenHeight() / 2) - floorVerticalOffset;
            //     DrawRectangle(0, 0, GetScreenWidth(), center, BLUE);
            //     DrawRectangle(0, center, GetScreenWidth(), GetScreenHeight(), GRAY);
            // }

            // Terain rendering
            {
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
                            hitDistance = Clamp(hitDistance, 0, cam.farPlaneDistance);

                            // Normalize distance to [0, 1]
                            float tBrightness = hitDistance / cam.farPlaneDistance;
                            uint8_t brightness = Lerp(255, 0, tBrightness);

                            float rayDirectionDeg = cam.fov * (floor(0.5 * GetScreenWidth()) - screenX) / GetScreenWidth();
                            float rayProjectionPositionInScreen = 0.5 * tanf(rayDirectionDeg * DEG2RAD) / tanf((0.5 * cam.fov) * DEG2RAD);
                            float objectHeight = round(GetScreenHeight() * cam.nearPlaneDistance / (hitDistance * cosf(rayDirectionDeg * DEG2RAD)));

                            // terrain rendering
                            {
                                float heightDelta = GetScreenHeight() - objectHeight;
                                float halfHeightDelta = heightDelta / 2;

                                float topY = (halfHeightDelta - floorVerticalOffset);
                                float bottomY = (topY + objectHeight);

                                DrawLine(
                                    screenX, topY, 
                                    screenX, bottomY, 
                                    ColorAlpha255(MY_PURPLE, brightness)
                                );
                            }
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
                DrawLineV(World[i].a, World[i].b, GRAY);
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

