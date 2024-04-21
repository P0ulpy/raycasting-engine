#include <raylib.h>
#define RAYGUI_IMPLEMENTATION
#include <raygui.h>
#include <rlImGui.h>

#include <imgui.h>
#include <imgui_stdlib.h>


constexpr int DefaultScreenWidth = 1000;
constexpr int DefaultScreenHeight = 650;

int main()
{
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(DefaultScreenWidth, DefaultScreenHeight, "wall-it");
    rlImGuiSetup(true);

    SetTargetFPS(144);
    
    float deltaTime = 0.0f;
    float lastFrameTime = GetTime();

    while (!WindowShouldClose())
    {
        float currentTime = GetTime();
        deltaTime = currentTime - lastFrameTime;
        lastFrameTime = currentTime;

        // Update

        BeginDrawing();

        ClearBackground(LIGHTGRAY);

        // Draw

        rlImGuiBegin();
            // GUI
        rlImGuiEnd();

        EndDrawing();
    }

    rlImGuiShutdown();
    CloseWindow();

    return 0;
}

