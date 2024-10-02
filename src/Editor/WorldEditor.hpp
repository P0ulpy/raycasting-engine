#pragma once

#include <imgui.h>
#include <imgui_internal.h>
#include <rlImGui.h>
#include <raylib.h>

#include <sstream>

#include "../World.hpp"
#include "../RaycastingCamera.hpp"
#include "../DrawingHelper.hpp"

class WorldEditor
{
public:
    WorldEditor(World& world, Vector2 target = { 0.f, 0.f })
        : world(world)
        , renderTexture(LoadRenderTexture(1920, 1080))
        , camera({ 
            .target = target,
            .rotation = 0.f,
            .zoom = 1.f,
        })
    {}

    ~WorldEditor()
    {
        UnloadRenderTexture(renderTexture);
    }

    void ResizeRenderTextureSize(int width, int height)
    {
        UnloadRenderTexture(renderTexture);
        renderTexture = LoadRenderTexture(width, height);

        camera.offset = { (float)renderTexture.texture.width / 2, (float)renderTexture.texture.height / 2 };
    }

    void DrawGUI()
    {
        RenderSectorsGui();
        RenderViewportGui();
    }

    void Update(float dt)
    {
        constexpr float MOUSE_DRAG_SENSITIVITY = 1.f;

        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
        {
            Vector2 delta = GetMouseDelta();
            delta = Vector2Scale(delta, -MOUSE_DRAG_SENSITIVITY / camera.zoom);
            camera.target = Vector2Add(camera.target, delta);
        }

        constexpr float MOUSE_ZOOM_SENSITIVITY = 1.f;

        float wheel = GetMouseWheelMove();
        if (wheel != 0)
        {
            Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);

            camera.offset = GetMousePosition();
            camera.target = mouseWorldPos;

            float scaleFactor = MOUSE_ZOOM_SENSITIVITY + ( 0.25f * fabsf(wheel));
            if (wheel < 0) scaleFactor = 1.0f / scaleFactor;
            camera.zoom = Clamp(camera.zoom * scaleFactor, 0.5f, 32.0f);
        }
    }

    void Render(RaycastingCamera& cam)
    {
        BeginTextureMode(renderTexture);

            ClearBackground(LIGHTGRAY);

            BeginMode2D(camera);

                DrawBackgroundGrid();

                DrawCam(cam);

                bool noSectorSelected = currentSelectedSector == NULL_SECTOR;

                for(const auto& [ sectorId, sector ] : world.Sectors)
                {
                    bool thisSectorSelected = !noSectorSelected && (sectorId == currentSelectedSector);

                    for(const auto& wall : sector.walls)
                    {
                        DrawWall(wall, noSectorSelected, thisSectorSelected);
                    }
                }

            EndMode2D();

            DrawUI();

        EndTextureMode();
    }

private:
    void DrawCam(const RaycastingCamera& cam) const
    {
        Vector2 camHeadingDirectionPoint = Vector2Add(cam.position, Vector2Scale(cam.Forward(), 15));
        Vector2 camRightDirectionPoint = Vector2Add(cam.position, Vector2Scale(cam.Right(), 15));
        DrawArrow(cam.position, camHeadingDirectionPoint, RED);
        DrawArrow(cam.position, camRightDirectionPoint, GREEN);
        
        DrawCircleV(cam.position, 3, BLUE);
    }

    void DrawBackgroundGrid() const
    {
        constexpr int32_t CELL_SIZE = 25;
        constexpr int32_t GRID_SIZE = 1000;

        const Color GridColor = ColorAlpha(GRAY, .1f);

        // Compute Grid Offset based on camera position

        int32_t rowOffset = (camera.target.x) / CELL_SIZE;
        int32_t startingRow = -(GRID_SIZE - rowOffset);
        int32_t endingRow = (GRID_SIZE + rowOffset);

        int32_t colOffset = (camera.target.y) / CELL_SIZE;
        int32_t startingCol = -(GRID_SIZE - colOffset);
        int32_t endingCol = (GRID_SIZE + colOffset);

        int32_t yRowBegin = startingCol * CELL_SIZE;
        int32_t yRowEnd = endingCol * CELL_SIZE;

        for (int32_t row = startingRow; row <= endingRow; ++row) 
        {
            int32_t x = CELL_SIZE * row;
            DrawLine(x, yRowBegin, x, yRowEnd, GridColor);
        }

        int32_t xColBegin = startingRow * CELL_SIZE;
        int32_t xColEnd = endingRow * CELL_SIZE;

        for (int32_t col = startingCol; col <= endingCol; ++col) 
        {
            int32_t y = CELL_SIZE * col;
            DrawLine(xColBegin, y, xColEnd, y, GridColor);
        }
        for (int32_t row = startingRow; row <= endingRow; ++row) 
        {
            int32_t x = CELL_SIZE * row;
            DrawLine(x, yRowBegin, x, yRowEnd, GridColor);
        }
    }

    void DrawWall(const Wall& wall, bool noSectorSelected, bool thisSectorSelected) const
    {
        Color segmentColor = wall.color;
        Color tikColor = RED;
        float thickness = 2.f;

        if(wall.toSector != NULL_SECTOR)
        {
            segmentColor = PURPLE;
        }

        if(!noSectorSelected && !thisSectorSelected)
        {
            segmentColor = ColorAlpha(segmentColor, 0.5);
            tikColor = ColorAlpha(tikColor, 0.5);
        }
        else if (thisSectorSelected)
        {
            thickness = 4.f;
        }

        const Segment& segment = wall.segment;

        DrawLineEx(segment.a, segment.b, thickness, segmentColor);

        Vector2 segmentDirection = Vector2Subtract(segment.a, segment.b);
        Vector2 segmentRightPerpDirection = Vector2Normalize({ -segmentDirection.y, segmentDirection.x });

        Vector2 tikPositionA = { (segment.a.x + segment.b.x) / 2, (segment.a.y + segment.b.y) / 2 };
        Vector2 tikPositionB = Vector2Add(tikPositionA, Vector2Scale(segmentRightPerpDirection, 10));

        DrawLineEx(tikPositionA, tikPositionB, thickness, tikColor);
    }

    void DrawUI()
    {
        // Top Right Axis
        {
            Vector2 axisCenter = {
                (float)renderTexture.texture.width - 100,
                50,
            };

            Vector2 xDirPoint = Vector2Add(axisCenter, Vector2Scale({ 1, 0 }, 70));
            Vector2 yDirPoint = Vector2Add(axisCenter, Vector2Scale({ 0, 1 }, 70));
            DrawArrow(axisCenter, xDirPoint, RED, 4, 15, 10);
            DrawArrow(axisCenter, yDirPoint, GREEN, 4, 15, 10);
        }

        // TODO : Get mouse position relative to ImguiWindow, and relative to 2D camera space

        {
            Vector2 posTextCenter = {
                (float)renderTexture.texture.width - 400,
                40,
            };

            Vector2 mousePos = GetMousePosition();

            std::stringstream posStr;
            posStr << "Cursor pos : [ " << (uint32_t)mousePos.x
                    << ", " << (uint32_t)mousePos.y << " ]";
            
            DrawText(posStr.str().c_str(), posTextCenter.x, posTextCenter.y, 20, BLACK);
        }
    }

    void RenderViewportGui()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::SetNextWindowSizeConstraints(ImVec2(renderTexture.texture.width, renderTexture.texture.height), ImVec2((float)GetScreenWidth(), (float)GetScreenHeight()));

        if (ImGui::Begin("World Editor", nullptr, ImGuiWindowFlags_NoScrollbar))
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
                
                std::string label = "Sector - " + std::to_string(sectorId);
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
            SectorID toSector = sector.walls[i].toSector;
            
            if(toSector != NULL_SECTOR)
            {
                ImGui::Text("[%zu] => %u", i, toSector);
            }
            else
            {
                ImGui::Text("[%zu] => NULL", i);
            }
        }
    }

private:
    World& world;
    RenderTexture2D renderTexture;

    // State
    SectorID currentSelectedSector = NULL_SECTOR;
    Camera2D camera = { 0 };
};