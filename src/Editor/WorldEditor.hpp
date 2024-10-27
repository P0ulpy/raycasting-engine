#pragma once

#include <raylib.h>

#include "Renderer/World.hpp"
#include "Renderer/RaycastingCamera.hpp"

class WorldEditor;

class WorldEditorDrawTool
{
public:
    explicit WorldEditorDrawTool(WorldEditor& worldEditor);

    void Update(float dt);
    void Render(RaycastingCamera& cam) const;

private:
    static constexpr KeyboardKey DragPrecisionModeKey = KEY_LEFT_SHIFT;
    bool isInDragPrecisionMode = false;

private:
    WorldEditor& worldEditor;
    bool isDragging = false;
    Vector2 dragStartPosition { 0, 0 };
    Vector2 dragEndPosition { 0, 0 };
};

class WorldEditor
{
public:
    WorldEditor(World& world, Vector2 target = { 0.f, 0.f });
    ~WorldEditor();
    
    void DrawGUI();
    void Update(float dt);
    void Render(RaycastingCamera& cam) const;

    void ResizeRenderTextureSize(int width, int height);

    int32_t GetGridCellSize() const;
    Vector2 ScreenToViewportPosition(Vector2 pos) const;
    Vector2 ScreenToWorldPosition(Vector2 viewportPos) const;

private:
    static void DrawCam(const RaycastingCamera& cam) ;
    void DrawBackgroundGrid() const;
    void DrawWall(const Wall& wall, bool noSectorSelected, bool thisSectorSelected) const;
    void DrawUI() const;

    void RenderViewportGui();
    void RenderSectorsGui();
    static void RenderSectorContentGui(Sector& sector);

private:
    World& world;
    RenderTexture2D renderTexture { 0 };

    WorldEditorDrawTool drawTool;
    Camera2D camera { 0 };

    SectorID currentSelectedSector = NULL_SECTOR;

    bool isViewportFocused = false;
    Vector2 viewportWindowOffset { 0, 0 };
    Vector2 viewportWindowSize { 0, 0 };

private:
    static constexpr float MouseZoomSensitivity = 1.f;
    static constexpr float MouseDragSensitivity = 1.f;
    static constexpr float MinZoom = 0.5f;
    static constexpr float MaxZoom = 32.0f;
    inline static const Color GridColor = ColorAlpha(GRAY, .1f);
    static constexpr int32_t GridSize = 10000;

private:
    friend class WorldEditorDrawTool;
};