
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

Vector2 WorldEditor::GetMouseViewportPosition() const
{
    Vector2 mousePos = GetMousePosition();
    Vector2 mousePosInViewport = Vector2Subtract(mousePos, viewportWindowOffset);

    Vector2 mousePosInRenderTexture = {
        mousePosInViewport.x * ((float)renderTexture.texture.width / viewportWindowSize.x),
        mousePosInViewport.y * ((float)renderTexture.texture.height / viewportWindowSize.y),
    };

    return mousePosInRenderTexture;
}

Vector2 WorldEditor::GetMouseWorldPosition() const
{
    Vector2 mousePos = GetMouseViewportPosition();
    Vector2 mouseWorldPos = GetScreenToWorld2D(mousePos, camera);

    return mouseWorldPos;
}

void WorldEditor::Update(float dt)
{
    if(isViewportFocused)
    {
        HandleInputs();
    }
}

void WorldEditor::Render(RaycastingCamera& cam)
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

            // Wall Drawing Render
            if(isDragging)
            {
                EndMode2D();
                    Vector2 mousePos = GetMouseViewportPosition();
                    DrawLine(mousePos.x, 0, mousePos.x, renderTexture.texture.height, GRAY);
                    DrawLine(0, mousePos.y, renderTexture.texture.width, mousePos.y, GRAY);

                BeginMode2D(camera);

                DrawLineV(dragStartPosition, dragEndPosition, ORANGE);
                DrawCircleV(dragStartPosition, 1, RED);
                DrawCircleV(dragEndPosition, 1, BLUE);
            }

        EndMode2D();

        DrawUI();

    EndTextureMode();
}

void WorldEditor::HandleInputs()
{
    constexpr float MOUSE_DRAG_SENSITIVITY = 1.f;

    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
    {
        Vector2 delta = GetMouseDelta();
        delta = Vector2Scale(delta, -MOUSE_DRAG_SENSITIVITY / camera.zoom);
        camera.target = Vector2Add(camera.target, delta);
    }

    constexpr float MOUSE_ZOOM_SENSITIVITY = 1.f;

    Vector2 mouseWorldPos = GetMouseWorldPosition();

    float wheel = GetMouseWheelMove();
    if (wheel != 0)
    {
        camera.offset = GetMouseViewportPosition();
        camera.target = mouseWorldPos;

        float scaleFactor = MOUSE_ZOOM_SENSITIVITY + ( 0.25f * fabsf(wheel));
        if (wheel < 0) scaleFactor = 1.0f / scaleFactor;

        camera.zoom = Clamp(camera.zoom * scaleFactor, MinZoom, MaxZoom);
    }

    // Wall Drawing Drag
    {
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            dragStartPosition = mouseWorldPos;
            isDragging = true;
        }
        else if(IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            dragEndPosition = mouseWorldPos;
        }
        //else if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        else if(isDragging)
        {
            float deltaLength = Vector2Length(Vector2Subtract(dragStartPosition, dragEndPosition));
            std::cout << "Delta Length : " << deltaLength << std::endl;
            isDragging = false;
        }
    }
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
//    int32_t cellSize = GetGridCellSize();
//    int32_t gridSize = cellSize * 100;
//
//    // Compute Grid Offset based on camera position
//
//    const int32_t rowOffset = (int32_t)camera.target.x / cellSize;
//    int32_t startingRow = -(gridSize - rowOffset);
//    int32_t endingRow = (gridSize + rowOffset);
//
//    const int32_t colOffset = (int32_t)camera.target.y / cellSize;
//    int32_t startingCol = -(gridSize - colOffset);
//    int32_t endingCol = (gridSize + colOffset);
//
//    int32_t yRowBegin = startingCol * cellSize;
//    int32_t yRowEnd = endingCol * cellSize;
//
//    for (int32_t row = startingRow; row <= endingRow; ++row)
//    {
//        int32_t x = cellSize * row;
//        DrawLine(x, yRowBegin, x, yRowEnd, GridColor);
//    }
//
//    int32_t xColBegin = startingRow * cellSize;
//    int32_t xColEnd = endingRow * cellSize;
//
//    for (int32_t col = startingCol; col <= endingCol; ++col)
//    {
//        int32_t y = cellSize * col;
//        DrawLine(xColBegin, y, xColEnd, y, GridColor);
//    }
//    for (int32_t row = startingRow; row <= endingRow; ++row)
//    {
//        int32_t x = cellSize * row;
//        DrawLine(x, yRowBegin, x, yRowEnd, GridColor);
//    }

    int32_t cellSize = GetGridCellSize();

    Vector2 center = {
            (float)renderTexture.texture.width / 2,
            (float)renderTexture.texture.height / 2
    };
    center = GetScreenToWorld2D(center, camera);

    Vector2 offset = {
        std::round(center.x / cellSize) * cellSize,
        std::round(center.y / cellSize) * cellSize
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

void WorldEditor::DrawUI()
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

        Vector2 mousePos = GetMouseViewportPosition();

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
    else if (camera.zoom <= 5.f)
        cellSize = 50;
    else if (camera.zoom <= 10.f)
        cellSize = 25;

    return cellSize;
}
