#pragma once

#include <imgui.h>
#include <imgui_internal.h>
#include <rlImGui.h>
#include <raylib.h>

#include "../World.hpp"
#include "../RaycastingCamera.hpp"

class WorldEditor
{
public:
    WorldEditor(World& world)
        : world(world)
        , renderTexture(LoadRenderTexture(1920, 1080))
    {}

    ~WorldEditor()
    {
        UnloadRenderTexture(renderTexture);
    }

    void ResizeRenderTextureSize(int width, int height)
    {
        UnloadRenderTexture(renderTexture);
        renderTexture = LoadRenderTexture(width, height);
    }

    void DrawGUI()
    {











        {
            ImGui::Begin("World Editor");
                ImGuiID worldEditorDockerSpace = ImGui::GetID("worldEditorDockerSpace");
                ImGui::DockSpace(worldEditorDockerSpace);
            ImGui::End();

            ImGui::SetNextWindowDockID(worldEditorDockerSpace, ImGuiCond_FirstUseEver);
            ImGui::Begin("Sectors");
                
                ImGui::Text("salut");

            ImGui::End();

            ImGui::SetNextWindowDockID(worldEditorDockerSpace, ImGuiCond_FirstUseEver);
            // ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
            // ImGui::SetNextWindowSizeConstraints(ImVec2(renderTexture.texture.width, renderTexture.texture.height), ImVec2((float)GetScreenWidth(), (float)GetScreenHeight()));

            if (ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoScrollbar))
            {
                // ImVec2 windowSize = ImGui::GetWindowSize();
                // if(windowSize.x != renderTexture.texture.width || windowSize.y != renderTexture.texture.height)
                // {
                //     ResizeRenderTextureSize(windowSize.x, windowSize.y);
                // }

                // rlImGuiImageRenderTextureFit(&renderTexture, true);
            }
            ImGui::End();

            // ImGui::PopStyleVar();
        }





        // ImGui::Begin("World Editor", nullptr, ImGuiWindowFlags_DockNodeHost);

        //     ImGui::Begin("Sectors");
        //         ImGui::Text("salut");
        //     ImGui::End();

        //     ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        //     ImGui::SetNextWindowSizeConstraints(ImVec2(renderTexture.texture.width, renderTexture.texture.height), ImVec2((float)GetScreenWidth(), (float)GetScreenHeight()));

        //     if (ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoScrollbar))
        //     {
        //         ImVec2 windowSize = ImGui::GetWindowSize();
        //         if(windowSize.x != renderTexture.texture.width || windowSize.y != renderTexture.texture.height)
        //         {
        //             ResizeRenderTextureSize(windowSize.x, windowSize.y);
        //         }

        //         rlImGuiImageRenderTextureFit(&renderTexture, true);
        //     }
        //     ImGui::End();

        //     ImGui::PopStyleVar();
        
        // ImGui::End();
    }

    void Render(RaycastingCamera& cam)
    {

    }

    void Update(float dt)
    {

    }

private:
    World& world;
    RenderTexture2D renderTexture;
};