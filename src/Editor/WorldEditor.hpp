#pragma once

#include <raylib.h>

#include "Renderer/World.hpp"
#include "Renderer/RaycastingCamera.hpp"

class WorldEditor
{
public:
    WorldEditor(World& world, Vector2 target = { 0.f, 0.f });
    ~WorldEditor();
    
    void DrawGUI();
    void Update(float dt);
    void Render(RaycastingCamera& cam);

    void ResizeRenderTextureSize(int width, int height);

private:
    void HandleInputs();

    static void DrawCam(const RaycastingCamera& cam) ;
    void DrawBackgroundGrid() const;
    void DrawWall(const Wall& wall, bool noSectorSelected, bool thisSectorSelected) const;
    void DrawUI();

    void RenderViewportGui();
    void RenderSectorsGui();
    static void RenderSectorContentGui(Sector& sector);

    [[nodiscard]] int32_t GetGridCellSize() const;
    [[nodiscard]] Vector2 GetMouseViewportPosition() const;
    [[nodiscard]] Vector2 GetMouseWorldPosition() const;

private:
    static constexpr float MinZoom = 0.5f;
    static constexpr float MaxZoom = 32.0f;
    inline static const Color GridColor = ColorAlpha(GRAY, .1f);
    static constexpr int32_t GridSize = 10000;

private:
    World& world;
    RenderTexture2D renderTexture { 0 };

    Camera2D camera { 0 };
    SectorID currentSelectedSector = NULL_SECTOR;

    bool isDragging = false;
    Vector2 dragStartPosition { 0, 0 };
    Vector2 dragEndPosition { 0, 0 };

    // GUI state
    bool isViewportFocused = false;
    Vector2 viewportWindowOffset { 0, 0 };
    Vector2 viewportWindowSize { 0, 0 };
};