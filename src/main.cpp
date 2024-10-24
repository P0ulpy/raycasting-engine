#include <raylib.h>
#include <raymath.h>
#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

#include <rlImGui.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_stdlib.h>

#include "Renderer/World.hpp"
#include "Renderer/RaycastingMath.hpp"
#include "Renderer/RaycastingCamera.hpp"
#include "Renderer/WorldRasterizer.hpp"

#include "Editor/ImGuiStyle.hpp"
#include "Editor/RaycastingCameraViewport.hpp"
#include "Editor/MiniMapViewport.hpp"
#include "Editor/RenderingOrchestrator.hpp"
#include "Editor/WorldEditor.hpp"

#include "Utils/ColorHelper.hpp"

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
    
    World world;

    RaycastingCamera cam = {
        { 550, 600 },
    };

    RaycastingCameraViewport cameraViewport(cam, 1920, 1080);
    WorldEditor worldEditor(world, cam.position);

    RenderingOrchestrator renderingOrchestrator(cameraViewport);

    while (!WindowShouldClose())
    {
        float deltaTime = GetFrameTime();
        // Inputs

        // Update
        
        {
            std::string windowTitle = "raycasting-engine [";
            windowTitle += std::to_string(GetFPS());
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
            
            worldEditor.Render(cam);
            renderingOrchestrator.Render(world, cam);

            // Draw GUI
            
            // Draw ImGUI

            rlImGuiBegin();

                ImGui::DockSpaceOverViewport();
                ApplicationMainMenuBar();

                cam.DrawGUI();
                cameraViewport.DrawGUI();
                renderingOrchestrator.DrawGUI();

                worldEditor.DrawGUI();

            rlImGuiEnd();

        EndDrawing();
    }

    rlImGuiShutdown();
    CloseWindow();

    return 0;
}

