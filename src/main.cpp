#include <raylib.h>
#include <raymath.h>
#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

#include <rlImGui.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_stdlib.h>
#include "ImGuiStyle.hpp"

#include "World.hpp"
#include "RaycastingMath.hpp"
#include "RaycastingCamera.hpp"
#include "ColorHelper.hpp"
#include "WorldRasterizer.hpp"
#include "RaycastingCameraViewport.hpp"
#include "MiniMapViewport.hpp"
#include "RenderingOrchestrator.hpp"
#include "Editor/WorldEditor.hpp"

constexpr int DefaultScreenWidth = 1720;
constexpr int DefaultScreenHeight = 880;

void ApplicationMainMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Menu"))
        {
            if (ImGui::MenuItem("Close"))
            {
                CloseWindow();
            }
                
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

int main()
{
    // init

    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(DefaultScreenWidth, DefaultScreenHeight, "raycasting-engine");
    SetExitKey(KEY_NULL);

    rlImGuiSetup(true);
    ImGuiIO& imguiIO = ImGui::GetIO();
    imguiIO.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    imguiIO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    imguiIO.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    SetupImGuiStyle();

    SetTargetFPS(144);

    /// Global
    
    float deltaTime = 0;
    float lastFrameTime = GetTime();
    
    World world;

    RaycastingCamera cam = {
        { 550, 600 },
    };

    RaycastingCameraViewport cameraViewport(cam, 1920, 1080);
    MiniMapViewport miniMapViewport(DefaultScreenWidth / 4, DefaultScreenWidth / 4); 
    WorldEditor worldEditor(world, cam.position);

    RenderingOrchestrator renderingOrchestrator(cameraViewport);
    
    while (!WindowShouldClose())
    {
        float currentTime = GetTime();
        deltaTime = currentTime - lastFrameTime;
        lastFrameTime = currentTime;

        // Inputs

        // Update

        {
            std::string windowTitle = "raycasting-engine [";
            windowTitle += std::to_string(1.0f / deltaTime);
            windowTitle += " FPS]";

            SetWindowTitle(windowTitle.c_str());
        }

        {
            // Update current sector
            uint32_t currentSectorId = FindSectorOfPoint(cam.position, world);
            if(NULL_SECTOR != currentSectorId)
            {
                cam.currentSectorId = currentSectorId;
            }
        }

        if(cameraViewport.IsFocused())
        {
            cam.Update(deltaTime);
            HideCursor();
        }
        else
        {
            ShowCursor();
        }

        worldEditor.Update(deltaTime);

        // Draw

        BeginDrawing();
            
            miniMapViewport.Render(world, cam);
            worldEditor.Render(cam);
            renderingOrchestrator.Render(world, cam);

            // Draw GUI
            
            // Draw ImGUI

            rlImGuiBegin();

                ImGui::DockSpaceOverViewport();
                ApplicationMainMenuBar();

                cam.DrawGUI();
                cameraViewport.DrawGUI();
                miniMapViewport.DrawGUI();
                renderingOrchestrator.DrawGUI();

                worldEditor.DrawGUI();

            rlImGuiEnd();

        EndDrawing();
    }

    rlImGuiShutdown();
    CloseWindow();

    return 0;
}

