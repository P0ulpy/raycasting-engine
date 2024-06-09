#pragma once

#include <raylib.h>

#include <stack>
#include <functional>

#include "RaycastingCamera.hpp"
#include "World.hpp"

struct RenderAreaYMinMax
{
    uint32_t max = 0;
    uint32_t min  = 0;
};

using RenderAreaYBoundaries = std::vector<RenderAreaYMinMax>;
struct RenderArea
{
    uint32_t xBegin { 0 };
    uint32_t xEnd   { 0 };
};

struct SectorRenderContext
{
    const SectorID sectorId { 0 };
    RenderArea renderArea;
};

struct RasterizeWorldContext 
{
    const World& world;
    const RaycastingCamera& cam;
    const uint32_t RenderTargetWidth; 
    const uint32_t RenderTargetHeight;
    const float FloorVerticalOffset;
    const float CamCurrentSectorElevationOffset;
    
    RenderAreaYBoundaries yBoundaries;
    std::stack<SectorRenderContext> renderStack;
};

RasterizeWorldContext InitRasterizeWorldContext(uint32_t RenderTargetWidth, uint32_t RenderTargetHeight, const World& world, const RaycastingCamera& cam);

void RasterizeWorldInTexture(const RenderTexture& renderTexture, const World& world, const RaycastingCamera& cam);

void RasterizeWorld(uint32_t RenderTargetWidth, uint32_t RenderTargetHeight, const World& world, const RaycastingCamera& cam);

struct RaycastHitData 
{
    float distance = std::numeric_limits<float>::max();
    Vector2 position;
    const Wall* wall = nullptr;
};

using RenderNextAreaBordersCallback = std::function<void(RasterizeWorldContext&, RenderAreaYMinMax&, const Sector&, const Sector&, uint32_t, float)>;

void RasterizeInRenderArea(RasterizeWorldContext& worldContext, SectorRenderContext renderContext, RenderNextAreaBordersCallback renderNextAreaBordersCallback);

void RenderNextAreaBorders(RasterizeWorldContext& worldContext, RenderAreaYMinMax& yMinMax, const Sector& currentSector, const Sector& nextSector, uint32_t x, float hitDistance);
struct CameraYLineData
{
    Vector2 top;
    Vector2 bottom;
    float depth;
    float normalizedDepth;
};

CameraYLineData ComputeCameraYAxis(
    const RaycastingCamera& cam, uint32_t renderTargetX, float hitDistance, 
    float FloorVerticalOffset, float CamCurrentSectorElevationOffset,
    uint32_t RenderTargetWidth, uint32_t RenderTargetHeight,
    uint32_t YHigh, uint32_t YLow,
    float topOffsetPercentage = 0, float bottomOffsetPercentage = 0
);

float ComputeVerticalOffset(const RaycastingCamera& cam, uint32_t RenderTargetHeight);
float ComputeElevationOffset(const RaycastingCamera& cam, const World& world, uint32_t RenderTargetHeight);

void RenderCameraYLine(CameraYLineData renderData, Color color, bool topBorder = true, bool bottomBorder = false);