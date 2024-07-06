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
        ImGui::Begin("World Editor");
            ImGuiID worldEditorDockerSpace = ImGui::GetID("worldEditorDockerSpace");
            ImGui::DockSpace(worldEditorDockerSpace);
        ImGui::End();

        ImGui::SetNextWindowDockID(worldEditorDockerSpace, ImGuiCond_FirstUseEver);
        RenderSectorsGui();

        ImGui::SetNextWindowDockID(worldEditorDockerSpace, ImGuiCond_FirstUseEver);
        RenderViewportGui();
    }

    void Update(float dt)
    {

    }

    void Render(RaycastingCamera& cam)
    {
        Camera2D minimapCamera = { 0 };
        minimapCamera.target = cam.position;
        minimapCamera.offset = { (float)renderTexture.texture.width / 2, (float)renderTexture.texture.height / 2 };
        minimapCamera.rotation = 0.0f;
        minimapCamera.zoom = 1.f;//zoom;

        BeginTextureMode(renderTexture);
            
            BeginMode2D(minimapCamera);

                ClearBackground(LIGHTGRAY);

                Vector2 camHeadingDirectionPoint = Vector2Add(cam.position, Vector2Scale(cam.Forward(), 50));
                DrawLineV(cam.position, camHeadingDirectionPoint, RED);
                DrawCircleV(cam.position, 10, GREEN);

                for(const auto& [ sectorId, sector ] : world.Sectors)
                {
                    bool sectorSelected = sectorId == currentSelectedSector;
                    
                    Vector2 selectionMin = { (float)renderTexture.texture.width, (float)renderTexture.texture.height };
                    Vector2 selectionMax = { 0, 0 };

                    for(const auto& wall : sector.walls)
                    {
                        Color color = WHITE;
                        if(sectorSelected)
                        {
                            color = RED;
                            if(wall.toSector != NULL_SECTOR)
                            {
                              color = BLUE;
                            }
                        }
                        else if(wall.toSector != NULL_SECTOR) color = PURPLE;

                        DrawLineV(wall.segment.a, wall.segment.b, color);

                        if(sectorSelected)
                        {
                            if(wall.segment.a.x < selectionMin.x)
                               selectionMin.x = wall.segment.a.x;
                            if(wall.segment.b.x < selectionMin.x)
                               selectionMin.x = wall.segment.b.x;

                            if(wall.segment.a.y < selectionMin.y)
                               selectionMin.y = wall.segment.a.y;
                            if(wall.segment.b.y < selectionMin.y)
                               selectionMin.y = wall.segment.b.y;

                            if(wall.segment.a.x > selectionMax.x)
                               selectionMax.x = wall.segment.a.x;
                            if(wall.segment.b.x > selectionMax.x)
                               selectionMax.x = wall.segment.b.x;

                            if(wall.segment.a.y > selectionMax.y)
                               selectionMax.y = wall.segment.a.y;
                            if(wall.segment.b.y > selectionMax.y)
                               selectionMax.y = wall.segment.b.y;
                        }
                    }

                    if(sectorSelected)
                    {
                        static const float padding = 10;

                        selectionMin.x -= padding;
                        selectionMin.y -= padding;
                        selectionMax.x += padding;
                        selectionMax.y += padding;

                        int width = selectionMax.x - selectionMin.x;
                        int height = selectionMax.y - selectionMin.y;

                        DrawRectangleLines(selectionMin.x, selectionMin.y, width, height, ColorAlpha(RED, .5f));
                    }
                }

            EndMode2D();

        EndTextureMode();
    }

private:
    void RenderViewportGui()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::SetNextWindowSizeConstraints(ImVec2(renderTexture.texture.width, renderTexture.texture.height), ImVec2((float)GetScreenWidth(), (float)GetScreenHeight()));

        if (ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoScrollbar))
        {
            ImVec2 windowSize = ImGui::GetWindowSize();
            if(windowSize.x != renderTexture.texture.width || windowSize.y != renderTexture.texture.height)
            {
                ResizeRenderTextureSize(windowSize.x, windowSize.y);
            }

            rlImGuiImageRenderTextureFit(&renderTexture, true);
        }
        ImGui::End();

        ImGui::PopStyleVar();
    }

    void RenderSectorsGui()
    {
        using namespace std::string_literals;

        ImGui::Begin("Sectors");
            
            for(auto& [ sectorId, sector ] : world.Sectors)
            {
                static const ImGuiTreeNodeFlags BaseSectorFlags = 
                    ImGuiTreeNodeFlags_OpenOnArrow 
                    | ImGuiTreeNodeFlags_OpenOnDoubleClick 
                    | ImGuiTreeNodeFlags_SpanAvailWidth;

                ImGuiTreeNodeFlags sectorFlags = BaseSectorFlags;
                if (currentSelectedSector == sectorId)
                {
                    sectorFlags |= ImGuiTreeNodeFlags_Selected;
                }
                
                std::string label = "Sector - "s + std::to_string(sectorId);
                bool sectorOpen = ImGui::TreeNodeEx(label.c_str(), sectorFlags);

                if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
                {
                    currentSelectedSector = sectorId;
                }

                if (sectorOpen)
                {
                    RenderSectorContentGui(sector);
                    ImGui::TreePop();
                }
            }
            
        ImGui::End();
    }

    void RenderSectorContentGui(Sector& sector)
    {
        ImGui::SliderFloat("zCeiling", &sector.zCeiling, -10, 10);
        ImGui::SliderFloat("zFloor", &sector.zFloor, -10, 10);

        for(size_t i = 0; i < sector.walls.size(); ++i)
        {
            ImGui::Text("[%d] => %d", i, sector.walls[i].toSector);
        }
    }

private:
    World& world;
    RenderTexture2D renderTexture;

    // State
    SectorID currentSelectedSector = NULL_SECTOR;
};