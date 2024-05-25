#pragma once

#include <imgui.h>
#include <rlImGui.h>

#include <raymath.h>
#include <raylib.h>

#include "World.hpp"
#include "RaycastingCamera.hpp"

class MiniMapViewport
{
public:
    MiniMapViewport(int width, int height)
        : renderTexture(LoadRenderTexture(width, height))
    {}

    void DrawGUI()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 5));
        ImGui::SetNextWindowSizeConstraints(ImVec2(renderTexture.texture.width, renderTexture.texture.height), ImVec2((float)GetScreenWidth(), (float)GetScreenHeight()));

        if (ImGui::Begin("MiniMap", nullptr, ImGuiWindowFlags_NoScrollbar))
        {
            focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows);

            if(ImGui::CollapsingHeader("Settings"))
            {
                ImGui::SliderFloat("Zoom", &zoom, 0.3f, 5.0f);
            }

            rlImGuiImageRenderTextureFit(&renderTexture, true);
            
            ImGui::End();
        }

        ImGui::PopStyleVar();
    }

    RenderTexture2D& GetRendertexture() { return renderTexture; }
    float GetZoom() const { return zoom; }

private:
    RenderTexture2D renderTexture { 0 };

    bool focused = false;
    float zoom = 1.f;
};

void RenderMinimap(RenderTexture2D& targetTexture, World& world, RaycastingCamera& cam, float zoom)
{
    Camera2D minimapCamera = { 0 };
    minimapCamera.target = cam.position;
    minimapCamera.offset = { (float)targetTexture.texture.width / 2, (float)targetTexture.texture.height / 2 };
    minimapCamera.rotation = 0.0f;
    minimapCamera.zoom = zoom;

    BeginTextureMode(targetTexture);
        
        BeginMode2D(minimapCamera);

            ClearBackground(LIGHTGRAY);

            Vector2 camHeadingDirectionPoint = Vector2Add(cam.position, Vector2Scale(cam.Forward(), 50));
            DrawLineV(cam.position, camHeadingDirectionPoint, RED);
            DrawCircleV(cam.position, 10, GREEN);

            for(const auto& [ key, sector ] : world.Sectors)
            {
                for(const auto& wall : sector.walls)
                {
                    DrawLineV(wall.segment.a, wall.segment.b, wall.color);
                }
            }

        EndMode2D();

    EndTextureMode();
}