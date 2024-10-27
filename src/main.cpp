#include <raylib.h>

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
#include "Editor/RenderingOrchestrator.hpp"
#include "Editor/WorldEditor.hpp"

constexpr int DefaultScreenWidth = 1720;
constexpr int DefaultScreenHeight = 880;

struct {
    bool renderingTool = false;
    bool cameraOptions = false;
    bool worldEditor = true;
} displayGuiStates;

void ApplicationMainMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Menu"))
        {
            ImGui::MenuItem("Rendering Tool", nullptr, &displayGuiStates.renderingTool);
            ImGui::MenuItem("Camera Options", nullptr, &displayGuiStates.cameraOptions);
            ImGui::MenuItem("World Editor", nullptr, &displayGuiStates.worldEditor);

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
    InitWindow(DefaultScreenWidth, DefaultScreenHeight, "raycasting-engine-editor");
    SetExitKey(KEY_NULL);

    rlImGuiSetup(true);
    ImGuiIO& imGuiIo = ImGui::GetIO();
    imGuiIo.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    imGuiIo.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    imGuiIo.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    SetupImGuiStyle();

    SetTargetFPS(144);
    
    World world;

    RaycastingCamera cam {
        { 550, 600 },
    };

    RaycastingCameraViewport cameraViewport(1920, 1080);
    WorldEditor worldEditor(world, cam.position);

    RenderingOrchestrator renderingOrchestrator(cameraViewport.GetRenderTexture());

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
            if(currentSectorId != NULL_SECTOR)
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

                cameraViewport.DrawGUI();

                if(displayGuiStates.cameraOptions)
                    cam.DrawGUI();
                if(displayGuiStates.renderingTool)
                    renderingOrchestrator.DrawGUI();
                if(displayGuiStates.worldEditor)
                    worldEditor.DrawGUI();

            rlImGuiEnd();

        EndDrawing();
    }

    rlImGuiShutdown();
    CloseWindow();

    return 0;
}

