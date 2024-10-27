#include "WorldEditor.hpp"

#include <iostream>

Vector2 PointToGridPoint(Vector2 point, int32_t cellSize)
{
    return {
        .x = std::round(point.x / (float)cellSize) * (float)cellSize,
        .y = std::round(point.y / (float)cellSize) * (float)cellSize
    };
}

Vector2 GetDragPointToGrid(Vector2 point, int32_t cellSize)
{
    constexpr float SnapDistanceRatio = 0.4f;

    Vector2 gridPoint = PointToGridPoint(point, cellSize);
    float distance = Vector2Length(Vector2Subtract(point, gridPoint));

    if(distance < (float)cellSize * SnapDistanceRatio)
    {
        return gridPoint;
    }
    else
    {
        return point;
    }
}

WorldEditorDrawTool::WorldEditorDrawTool(WorldEditor& worldEditor)
    : worldEditor(worldEditor)
{}

void WorldEditorDrawTool::Update(float dt)
{
    isInDragPrecisionMode = IsKeyDown(DragPrecisionModeKey);
    Vector2 mouseWorldPos = worldEditor.ScreenToWorldPosition(GetMousePosition());
    uint32_t cellSize = worldEditor.GetGridCellSize();

    if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        if(isInDragPrecisionMode)
            dragStartPosition = mouseWorldPos;
        else
            dragStartPosition = GetDragPointToGrid(mouseWorldPos, cellSize);

        isDragging = true;
    }
    else if(IsMouseButtonDown(MOUSE_BUTTON_LEFT))
    {
        if(isInDragPrecisionMode)
            dragEndPosition = mouseWorldPos;
        else
            dragEndPosition = GetDragPointToGrid(mouseWorldPos, cellSize);
    }
    // else if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
    else if(isDragging)
    {
        float deltaLength = Vector2Length(Vector2Subtract(dragStartPosition, dragEndPosition));
        std::cout << "Delta Length : " << deltaLength << '\n';
        isDragging = false;
    }
}

void WorldEditorDrawTool::Render(RaycastingCamera &cam) const
{
    if(!isDragging) return;

    Vector2 mousePos = worldEditor.ScreenToViewportPosition(GetMousePosition());
    DrawLine(mousePos.x, 0, mousePos.x, worldEditor.renderTexture.texture.height, GRAY);
    DrawLine(0, mousePos.y, worldEditor.renderTexture.texture.width, mousePos.y, GRAY);

    BeginMode2D(worldEditor.camera);

        DrawLineV(dragStartPosition, dragEndPosition, ORANGE);
        DrawCircleV(dragStartPosition, 1, RED);
        DrawCircleV(dragEndPosition, 1, BLUE);

    EndMode2D();
}
