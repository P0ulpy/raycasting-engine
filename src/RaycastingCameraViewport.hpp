#pragma once

#include <imgui.h>
#include <rlImGui.h>

#include "RaycastingCamera.hpp"

class RaycastingCameraViewport
{
public:
    RaycastingCameraViewport(RaycastingCamera& camera)
        : cam(camera)
    {}

    void DrawGUI()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::SetNextWindowSizeConstraints(ImVec2(400, 400), ImVec2((float)GetScreenWidth(), (float)GetScreenHeight()));

        if (ImGui::Begin("3D View", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_MenuBar))
        {
            MenuBar();

            if(autoResize)
            {
                ImVec2 windowSize = ImGui::GetWindowSize();
                if(windowSize.x != cam.renderTexture.texture.width || windowSize.y != cam.renderTexture.texture.height)
                {
                    cam.UpdateRenderTextureSize(windowSize.x, windowSize.y);
                }
            }

            focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows);
            // draw the view
            rlImGuiImageRenderTextureFit(&cam.renderTexture, true);
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }

    bool IsFocused() const
    {
        return focused;
    }

private:
    void MenuBar()
    {
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("Resolution"))
            {
                if(ImGui::MenuItem("1920x1080 (16:9)"))
                {
                    cam.UpdateRenderTextureSize(1920, 1080);
                    autoResize = false;
                }
                if(ImGui::MenuItem("720x480 (4:3)"))
                {
                    cam.UpdateRenderTextureSize(720, 480);
                    autoResize = false;
                }
                if(ImGui::MenuItem("Fit to window size"))
                {
                    ImVec2 windowSize = ImGui::GetWindowSize();
                    if(windowSize.x != cam.renderTexture.texture.width || windowSize.y != cam.renderTexture.texture.height)
                    {
                        cam.UpdateRenderTextureSize(windowSize.x, windowSize.y);
                    }

                    autoResize = true;
                }

                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
    }

private:
    RaycastingCamera& cam;

    bool mouseLocked = false;
    bool focused     = false;
    bool autoResize  = false;
};