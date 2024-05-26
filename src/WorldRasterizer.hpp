#pragma once

#include <raylib.h>

#include <limits>
#include <stack>
#include <vector>

#include "RaycastingCamera.hpp"
#include "World.hpp"

struct RenderAreaYBoundary
{
    uint32_t yHigh = 0;
    uint32_t yLow  = 0;
};

using RenderAreaYBoundaries = std::vector<RenderAreaYBoundary>;
struct RenderArea
{
    uint32_t xBegin;
    uint32_t xEnd;
    RenderAreaYBoundaries* yBoundaries = nullptr;
};

struct SectorRenderContext
{
    const World* world;
    const Sector* sector;
    RenderArea renderArea;
};

void RasterizeWorld(const World& world, const RaycastingCamera& cam);

struct RaycastHitData 
{
    float distance = std::numeric_limits<float>::max();
    Vector2 position;
    const Wall* wall = nullptr;
};

void RasterizeInRenderArea(SectorRenderContext renderContext, std::stack<SectorRenderContext>& renderStack, const RaycastingCamera& cam);

struct CameraYAxisData
{
    Vector2 top;
    Vector2 bottom;
    float depth;
    float normalizedDepth;
};

CameraYAxisData ComputeCameraYAxis(
    const RaycastingCamera& cam, uint32_t renderTargetX, float hitDistance, float floorVerticalOffset,
    uint32_t YHigh, uint32_t YLow,
    float topOffsetPercentage = 0, float bottomOffsetPercentage = 0
);

void RenderCameraYAxis(CameraYAxisData renderData, Color color, bool topBorder = true, bool bottomBorder = false);