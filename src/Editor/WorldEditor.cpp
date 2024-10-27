
#include "WorldEditor.hpp"
#include "Utils/DrawingHelper.hpp"

#include <imgui.h>
#include <imgui_internal.h>
#include <rlImGui.h>
#include <sstream>
#include <iostream>

WorldEditor::WorldEditor(World& world, Vector2 target)
    : world(world)
    , renderTexture(LoadRenderTexture(1920, 1080))
    , camera({ 
        .target = target,
        .rotation = 0.f,
        .zoom = 1.f,
    })
    , drawTool(*this)
{}

WorldEditor::~WorldEditor()
{
    UnloadRenderTexture(renderTexture);
}

void WorldEditor::ResizeRenderTextureSize(int width, int height)
{
    UnloadRenderTexture(renderTexture);
    renderTexture = LoadRenderTexture(width, height);

    camera.offset = { (float)renderTexture.texture.width / 2, (float)renderTexture.texture.height / 2 };
}

void WorldEditor::DrawGUI()
{
    RenderSectorsGui();
    RenderViewportGui();
}

Vector2 WorldEditor::ScreenToViewportPosition(Vector2 pos) const
{
    Vector2 posInViewport = Vector2Subtract(pos, viewportWindowOffset);

    Vector2 posInRenderTexture = {
        posInViewport.x * ((float)renderTexture.texture.width / viewportWindowSize.x),
        posInViewport.y * ((float)renderTexture.texture.height / viewportWindowSize.y),
    };

    return posInRenderTexture;
}

Vector2 WorldEditor::ScreenToWorldPosition(Vector2 viewportPos) const
{
    Vector2 pos = ScreenToViewportPosition(viewportPos);
    return GetScreenToWorld2D(pos, camera);
}

void WorldEditor::Update(float dt)
{
    if(isViewportFocused)
    {
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
        {
            Vector2 delta = GetMouseDelta();
            delta = Vector2Scale(delta, - MouseDragSensitivity / camera.zoom);
            camera.target = Vector2Add(camera.target, delta);
        }

        Vector2 mouseWorldPos = ScreenToWorldPosition(GetMousePosition());

        float wheel = GetMouseWheelMove();
        if (wheel != 0)
        {
            camera.offset = ScreenToViewportPosition(GetMousePosition());
            camera.target = mouseWorldPos;

            float scaleFactor = MouseZoomSensitivity + (0.25f * fabsf(wheel));
            if (wheel < 0) scaleFactor = 1.0f / scaleFactor;

            camera.zoom = Clamp(camera.zoom * scaleFactor, MinZoom, MaxZoom);
        }

        drawTool.Update(dt);
    }
}

void WorldEditor::Render(RaycastingCamera& cam) const
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

        drawTool.Render(cam);

    EndTextureMode();
}

void WorldEditor::DrawCam(const RaycastingCamera& cam)
{
    Vector2 camHeadingDirectionPoint = Vector2Add(cam.position, Vector2Scale(cam.Forward(), 15));
    Vector2 camRightDirectionPoint = Vector2Add(cam.position, Vector2Scale(cam.Right(), 15));
    DrawArrow(cam.position, camHeadingDirectionPoint, RED);
    DrawArrow(cam.position, camRightDirectionPoint, GREEN);
    
    DrawCircleV(cam.position, 3, BLUE);
}

void WorldEditor::DrawBackgroundGrid() const
{
    int32_t cellSize = GetGridCellSize();

    Vector2 center = {
            (float)renderTexture.texture.width / 2,
            (float)renderTexture.texture.height / 2
    };
    center = GetScreenToWorld2D(center, camera);

    Vector2 offset = {
        std::round(center.x / (float)cellSize) * (float)cellSize,
        std::round(center.y / (float)cellSize) * (float)cellSize
    };

    int32_t xStart = ((int32_t)offset.x - GridSize);
    int32_t xEnd = ((int32_t)offset.x + GridSize);
    int32_t yStart = ((int32_t)offset.y - GridSize);
    int32_t yEnd = ((int32_t)offset.y + GridSize);

    for(int32_t pos = xStart; pos <= xEnd; pos += cellSize)
    {
        DrawLine(pos, yStart, pos, yEnd, GridColor);
    }

    for(int32_t pos = yStart; pos <= yEnd; pos += cellSize)
    {
        DrawLine(xStart, pos, xEnd, pos, GridColor);
    }
}

void WorldEditor::DrawWall(const Wall& wall, bool noSectorSelected, bool thisSectorSelected) const
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

void WorldEditor::DrawUI() const
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

    {
        Vector2 posTextCenter = {
            (float)renderTexture.texture.width - 400,
            40,
        };

        Vector2 mousePos = ScreenToViewportPosition(GetMousePosition());

        std::stringstream posStr;
        posStr << "Cursor pos : [ " << (uint32_t)mousePos.x << ", " << (uint32_t)mousePos.y << " ]";
        
        DrawText(posStr.str().c_str(), posTextCenter.x, posTextCenter.y, 20, BLACK);
    }
}

void WorldEditor::RenderViewportGui()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::SetNextWindowSizeConstraints(ImVec2(renderTexture.texture.width, renderTexture.texture.height), ImVec2((float)GetScreenWidth(), (float)GetScreenHeight()));

    if (ImGui::Begin("World Editor", nullptr, ImGuiWindowFlags_NoScrollbar))
    {
        ImVec2 region = ImGui::GetContentRegionAvail();

        if((int)region.x != renderTexture.texture.width || (int)region.y != renderTexture.texture.height)
        {
            ResizeRenderTextureSize((int)region.x, (int)region.y);
        }

        rlImGuiImageRenderTextureFit(&renderTexture, true);

        ImVec2 viewportRecMin = ImGui::GetItemRectMin();
        ImVec2 viewportRecMax = ImGui::GetItemRectMax();
        viewportWindowOffset = { viewportRecMin.x, viewportRecMin.y };
        viewportWindowSize = {
            viewportRecMax.x - viewportRecMin.x,
            viewportRecMax.y - viewportRecMin.y
        };
    }

    isViewportFocused = ImGui::IsWindowFocused();

    ImGui::End();

    ImGui::PopStyleVar();
}

void WorldEditor::RenderSectorsGui()
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

void WorldEditor::RenderSectorContentGui(Sector& sector)
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

int32_t WorldEditor::GetGridCellSize() const
{
//    constexpr int32_t MIN_CELL_SIZE = 10;
//    constexpr int32_t MAX_CELL_SIZE = 100;
//    constexpr int32_t CELL_SIZE_ROUND = 25;
//
//    float zoom = Normalize(camera.zoom, MinZoom, MaxZoom);
//    int32_t size = Lerp(MAX_CELL_SIZE, MIN_CELL_SIZE, zoom);
//
//    if(size > 0 && size % CELL_SIZE_ROUND != 0)
//    {
//        size = (int32_t) ( ((float)size / (float)CELL_SIZE_ROUND) * (float)CELL_SIZE_ROUND );
//    }
//
//    return size;

    int32_t cellSize = 10;

    if(camera.zoom <= 1.f)
        cellSize = 100;
    else if (camera.zoom <= 3.f)
        cellSize = 50;
    else if (camera.zoom <= 10.f)
        cellSize = 25;

    return cellSize;
}
