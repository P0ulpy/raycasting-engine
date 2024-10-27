#pragma once

#include <raylib.h>
#include <stack>
#include <functional>

#include "Renderer/RaycastingCamera.hpp"
#include "Renderer/World.hpp"

template <typename T>
struct MinMax
{
    T max { 0 };
    T min { 0 };
};

using MinMaxUint32 = MinMax<uint32_t>;

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
    const World* world          { nullptr };
    const RaycastingCamera* cam { nullptr };
    uint32_t RenderTargetWidth  { 0 };
    uint32_t RenderTargetHeight { 0 };
    float FloorVerticalOffset               { 0.f };
    float CamCurrentSectorElevationOffset   { 0.f };
    
    uint32_t currentRenderItr { 0 };
    std::vector<MinMaxUint32> yBoundaries;
    std::stack<SectorRenderContext> renderStack;
};

void RasterizeInRenderArea(RasterizeWorldContext& worldContext, SectorRenderContext renderContext);

void RenderNextAreaBorders(RasterizeWorldContext& worldContext, MinMaxUint32& yMinMax, const Sector& currentSector, const Sector& nextSector, uint32_t x, float hitDistance);
struct CameraYLineData
{
    Vector2 top;
    Vector2 bottom;
    float depth = 0;
    float normalizedDepth = 0;
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

class WorldRasterizer
{
public:
    WorldRasterizer() = default;
    WorldRasterizer(WorldRasterizer&& other) = delete;

    WorldRasterizer(uint32_t renderTargetWidth, uint32_t renderTargetHeight, const World& world, const RaycastingCamera& cam);
    void Reset(uint32_t renderTargetWidth, uint32_t renderTargetHeight, const World& world, const RaycastingCamera& cam);

    bool IsRenderIterationRemains() const;

    void RasterizeWorldInTexture(const RenderTexture& renderTexture);
    void RasterizeWorld();
    void RenderIteration();

    const RasterizeWorldContext& GetContext() const { return ctx; }

private:
    RasterizeWorldContext ctx;
};