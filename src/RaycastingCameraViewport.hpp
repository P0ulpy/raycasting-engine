#pragma once

#include <imgui.h>
#include <rlImGui.h>

#include "RaycastingCamera.hpp"

class RaycastingCameraViewport
{
public:
    RaycastingCameraViewport(RaycastingCamera& camera, uint32_t RenderTextureWidth, uint32_t RenderTextureHeight)
        : cam(camera)
        , renderTexture(LoadRenderTexture(RenderTextureWidth, RenderTextureHeight))
    {}

    ~RaycastingCameraViewport()
    {
        UnloadRenderTexture(renderTexture);
    }

    void UpdateRenderTextureSize(int width, int height)
    {
        UnloadRenderTexture(renderTexture);
        renderTexture = LoadRenderTexture(width, height);
    }

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
                if(windowSize.x != renderTexture.texture.width || windowSize.y != renderTexture.texture.height)
                {
                    UpdateRenderTextureSize(windowSize.x, windowSize.y);
                }
            }

            focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows);
            // draw the view
            rlImGuiImageRenderTextureFit(&renderTexture, true);
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }

    bool IsFocused() const
    {
        return focused;
    }

    const RenderTexture2D& GetRenderTexture() const { return renderTexture; }

private:
    void MenuBar()
    {
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("Resolution"))
            {
                if(ImGui::MenuItem("1920x1080 (16:9)"))
                {
                    UpdateRenderTextureSize(1920, 1080);
                    autoResize = false;
                }
                if(ImGui::MenuItem("720x480 (4:3)"))
                {
                    UpdateRenderTextureSize(720, 480);
                    autoResize = false;
                }
                if(ImGui::MenuItem("Fit to window size"))
                {
                    ImVec2 windowSize = ImGui::GetWindowSize();
                    if(windowSize.x != renderTexture.texture.width || windowSize.y != renderTexture.texture.height)
                    {
                        UpdateRenderTextureSize(windowSize.x, windowSize.y);
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
    RenderTexture2D renderTexture;

    bool mouseLocked = false;
    bool focused     = false;
    bool autoResize  = false;
};