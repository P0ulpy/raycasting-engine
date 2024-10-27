#pragma once

#include <imgui.h>
#include <rlImGui.h>
#include <iostream>

#include "Renderer/RaycastingCamera.hpp"

class RaycastingCameraViewport
{
public:
    RaycastingCameraViewport(int32_t RenderTextureWidth, int32_t RenderTextureHeight)
        : renderTexture(LoadRenderTexture(RenderTextureWidth, RenderTextureHeight))
    {}

    ~RaycastingCameraViewport()
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
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(3, 0));
        ImGui::SetNextWindowSizeConstraints(ImVec2(400, 400), ImVec2((float)GetScreenWidth(), (float)GetScreenHeight()));

        if (ImGui::Begin("3D View", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_MenuBar))
        {
            MenuBar();

            if(autoResize)
            {
                ImVec2 windowSize = ImGui::GetWindowSize();
                if(windowSize.x != renderTexture.texture.width || windowSize.y != renderTexture.texture.height)
                {
                    ResizeRenderTextureSize(windowSize.x, windowSize.y);
                }
            }

            // draw the view
            rlImGuiImageRenderTextureFit(&renderTexture, true);

            if(ImGui::IsWindowFocused() && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                ImVec2 viewportRecMin = ImGui::GetItemRectMin();
                ImVec2 viewportRecMax = ImGui::GetItemRectMax();

                Vector2 mousePos = GetMousePosition();
                if(mousePos.x >= viewportRecMin.x && mousePos.x <= viewportRecMax.x
                    && mousePos.y >= viewportRecMin.y && mousePos.y <= viewportRecMax.y
                )
                {
                    mouseFocused = true;
                }
            }

            if(IsFocused() && IsKeyPressed(KEY_ESCAPE))
            {
                mouseFocused = false;
            }
        }
        ImGui::End();

        ImGui::PopStyleVar();
    }

    bool IsFocused() const
    {
        return mouseFocused;
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
                    ResizeRenderTextureSize(1920, 1080);
                    autoResize = false;
                }
                if(ImGui::MenuItem("720x480 (4:3)"))
                {
                    ResizeRenderTextureSize(720, 480);
                    autoResize = false;
                }
                if(ImGui::MenuItem("Fit to window size"))
                {
                    ImVec2 windowSize = ImGui::GetWindowSize();
                    if(windowSize.x != renderTexture.texture.width || windowSize.y != renderTexture.texture.height)
                    {
                        ResizeRenderTextureSize(windowSize.x, windowSize.y);
                    }

                    autoResize = true;
                }

                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
    }

private:
    RenderTexture2D renderTexture;

    bool mouseFocused = false;
    bool autoResize = false;
};