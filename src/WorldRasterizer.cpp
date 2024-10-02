#include "WorldRasterizer.hpp"

#include "RaycastingMath.hpp"
#include "ColorHelper.hpp"

#include <limits>
#include <iostream>
#include <unordered_map>
#include <vector>

RasterizeWorldContext InitRasterizeWorldContext(uint32_t RenderTargetWidth, uint32_t RenderTargetHeight, const World& world, const RaycastingCamera& cam)
{
    RasterizeWorldContext context = {
        .world = world,
        .cam = cam,
        .RenderTargetWidth = RenderTargetWidth,
        .RenderTargetHeight = RenderTargetHeight,
        .FloorVerticalOffset = ComputeVerticalOffset(cam, RenderTargetHeight),
        .CamCurrentSectorElevationOffset = 0, // TODO : ComputeElevationOffset(cam, world, RenderTargetHeight),
    };

    context.yBoundaries.resize(RenderTargetWidth);
    
    for (auto& bound : context.yBoundaries) 
    {
        bound.max = RenderTargetHeight;
        bound.min  = 0;
    }
    
    // assert(world.Sectors.contains(cam.currentSector), "Try to InitRasterizeWorldContext with an invalid SectorID");

    context.renderStack.push({
        .sectorId = cam.currentSectorId,
        .renderArea = {
            .xBegin = 0,
            .xEnd = RenderTargetWidth - 1,
        }
    });

    return context;
}

void RasterizeWorldInTexture(const RenderTexture& renderTexture, const World &world, const RaycastingCamera &cam)
{
    BeginTextureMode(renderTexture);
        RasterizeWorld(renderTexture.texture.width, renderTexture.texture.height, world, cam);
    EndTextureMode();
}

void RasterizeWorld(uint32_t RenderTargetWidth, uint32_t RenderTargetHeight, const World& world, const RaycastingCamera& cam)
{
    RasterizeWorldContext rasterizeContext = InitRasterizeWorldContext(RenderTargetWidth, RenderTargetHeight, world, cam);

    ClearBackground(MY_BLACK);

    for(size_t i = 0;
        0 < rasterizeContext.renderStack.size() && i < cam.maxRenderItr;
        ++i
    )
    {
        RasterizeInRenderArea(rasterizeContext, rasterizeContext.renderStack.top(), RenderNextAreaBorders);
    }
}

void RasterizeInRenderArea(RasterizeWorldContext& worldContext, SectorRenderContext renderContext, RenderNextAreaBordersCallback renderNextAreaBordersCallback)
{
    std::unordered_map<SectorID, SectorRenderContext> renderAreaToPushInStack;

    auto& [ world, cam, RenderTargetWidth, RenderTargetHeight, FloorVerticalOffset, CamCurrentSectorElevationOffset, yBoundaries, renderStack ] = worldContext;
    const auto& [ sectorId, renderArea ] = renderContext;
    
    const Sector& currentSector = world.Sectors.at(sectorId);

    for(uint32_t x = renderArea.xBegin; x <= renderArea.xEnd; ++x)
    {
        RenderAreaYMinMax& yMinMax = yBoundaries.at(x);

        float rayAngle = RayAngleforScreenXCam(x, cam, RenderTargetWidth);

        RasterRay rasterRay = {
            .position = cam.position,
            .direction = Vector2DirectionFromAngle((rayAngle * DEG2RAD) + cam.yaw),
        };

        RaycastHitData bestHitData;

        for(const Wall& wall : currentSector.walls)
        {
            HitInfo hitInfo;
            if(RayToSegmentCollision(rasterRay, wall.segment, hitInfo))
            {
                if(wall.toSector != NULL_SECTOR 
                    && PointSegmentSide(cam.position, wall.segment.a, wall.segment.b) <= 0)
                {
                    continue;
                }

                if(bestHitData.distance > hitInfo.distance)
                {
                    bestHitData = {
                        .distance = hitInfo.distance,
                        .position = hitInfo.position,
                        .wall = &wall,
                    };
                }
            }
        }

        if(bestHitData.distance < std::numeric_limits<float>::max())
        {
            // Floor / ceiling rendering
            {
                // // Draw floor
                // float centerY = ((RenderTargetHeight / 2) - FloorVerticalOffset) + CamCurrentSectorElevationOffset;
                // DrawLine(x, yMinMax.min, x, centerY, currentSector.ceilingColor);
                // // Draw Ceiling
                // DrawLine(x, centerY, x, yMinMax.max, currentSector.floorColor);
            }

            // Means this is a slid wall
            if(bestHitData.wall->toSector == NULL_SECTOR)
            {
                CameraYLineData cameraWallYData = 
                    ComputeCameraYAxis(cam, x, bestHitData.distance, 
                        FloorVerticalOffset, CamCurrentSectorElevationOffset,
                        RenderTargetWidth, RenderTargetHeight,
                        yMinMax.max,
                        yMinMax.min,
                        currentSector.zFloor, currentSector.zCeiling
                    );

                RenderCameraYLine(cameraWallYData, bestHitData.wall->color);
            }
            else
            {
                // assert(world.Sectors.contains(bestHitData.wall->toSector), "Try to render a next sector with an invalid SectorID");

                const SectorID nextSectorId = bestHitData.wall->toSector;
                const Sector& nextSector = world.Sectors.at(nextSectorId);
                
                renderNextAreaBordersCallback(worldContext, yMinMax, currentSector, nextSector, x, bestHitData.distance);

                // Create / update NextRenderArea

                if(!renderAreaToPushInStack.contains(nextSectorId))
                {
                    SectorRenderContext nextRenderAreaContext = {
                        .sectorId = nextSectorId,
                        .renderArea = {
                            .xBegin = x,
                            .xEnd = x
                        },
                    };

                    renderAreaToPushInStack.emplace(nextSectorId, nextRenderAreaContext);
                }
                else
                {
                    renderAreaToPushInStack.at(nextSectorId).renderArea.xEnd = x;
                }
            
                // Draw a Purple placeholder where next sector will be drawn
                DrawLineV({(float)x, (float)yMinMax.min}, { (float)x, (float)yMinMax.max }, PURPLE);
            }
        }
    }

    // current render is over pop it
    renderStack.pop();

    for(const auto& [ key, renderAreaCtx ] : renderAreaToPushInStack)
    {
        renderStack.push(renderAreaCtx);
    }
}

void RenderNextAreaBorders(RasterizeWorldContext& worldContext, RenderAreaYMinMax& yMinMax, const Sector& currentSector, const Sector& nextSector, uint32_t x, float hitDistance)
{
    // TODO : 
    // zCeilling shloud not be < to current zFloor
    // And this is the same in the other way
    // zFloor should not be > to current zCeiling
 
    // Top Border
    {
        bool nextSectCelingHigher = nextSector.zCeiling >= currentSector.zCeiling;

        const Sector& zSizesSector = (nextSectCelingHigher) ? currentSector : nextSector;

        CameraYLineData topBorderLineData = ComputeCameraYAxis(worldContext.cam, x, hitDistance, 
            worldContext.FloorVerticalOffset, worldContext.CamCurrentSectorElevationOffset,
            worldContext.RenderTargetWidth, worldContext.RenderTargetHeight,
            yMinMax.max, yMinMax.min,
            0, zSizesSector.zCeiling
        );

        if(!nextSectCelingHigher)
        {
            bool topEdge = !nextSectCelingHigher;
            RenderCameraYLine(topBorderLineData, nextSector.topBorderColor, topEdge, true);
        }

        // Apply Y min
        yMinMax.min = topBorderLineData.bottom.y;
    }

    // Bottom Border
    {
        bool nextSectFloorHigher = nextSector.zFloor >= currentSector.zFloor;

        const Sector& zSizesSector = (nextSectFloorHigher) ? currentSector : nextSector;

        CameraYLineData bottomBorderLineData = ComputeCameraYAxis(worldContext.cam, x, hitDistance, 
            worldContext.FloorVerticalOffset, worldContext.CamCurrentSectorElevationOffset,
            worldContext.RenderTargetWidth, worldContext.RenderTargetHeight,
            yMinMax.max, yMinMax.min,
            zSizesSector.zFloor, 0
        );

        if(!nextSectFloorHigher)
        {
            bool bottomEdge = !nextSectFloorHigher;
            RenderCameraYLine(bottomBorderLineData, nextSector.bottomBorderColor, true, bottomEdge);
        }

        // Apply Y max
        yMinMax.max = bottomBorderLineData.top.y;
    }
}

float ComputeVerticalOffset(const RaycastingCamera& cam, uint32_t RenderTargetHeight)
{
    return round(0.5f * RenderTargetHeight * (tanf(cam.pitch)) / tanf(0.5f * cam.fovVectical));
}

float ComputeElevationOffset(const RaycastingCamera& cam, const World& world, uint32_t RenderTargetHeight)
{ 
    // TODO : const float OneSectorHeight = RenderTargetHeight * cam.nearPlaneDistance;
    
    const Sector& currentSector = world.Sectors.at(cam.currentSectorId);
    return Lerp((float)RenderTargetHeight, 0.f, currentSector.zFloor);
}

CameraYLineData ComputeCameraYAxis(
    const RaycastingCamera& cam, uint32_t renderTargetX, float hitDistance, 
    float FloorVerticalOffset, float CamCurrentSectorElevationOffset,
    uint32_t RenderTargetWidth, uint32_t RenderTargetHeight,
    uint32_t YHigh, uint32_t YLow,
    float topOffsetPercentage, float bottomOffsetPercentage
)
{
    const float depth = Clamp(hitDistance, 0, cam.farPlaneDistance);
    // Normalize distance to [0, 1]
    const float normalizedDepth = depth / cam.farPlaneDistance;

    const float rayDirectionDeg = cam.fov * (floor(0.5 * RenderTargetWidth) - renderTargetX) / RenderTargetWidth;
    const float rayProjectionPositionInScreen = 0.5 * tanf(rayDirectionDeg * DEG2RAD) / tanf((0.5 * cam.fov) * DEG2RAD);
    const float objectHeight = round(RenderTargetHeight * cam.nearPlaneDistance / (depth * cosf(rayDirectionDeg * DEG2RAD)));

    // Rendering
    const float heightDelta = RenderTargetHeight - objectHeight;
    const float halfHeightDelta = heightDelta / 2;
    
    const float fullSizeTopY = (halfHeightDelta - FloorVerticalOffset);
    
    float topY    = fullSizeTopY + (objectHeight * topOffsetPercentage);
    float bottomY = (fullSizeTopY + objectHeight) - (objectHeight * bottomOffsetPercentage);

    topY += CamCurrentSectorElevationOffset;
    bottomY += CamCurrentSectorElevationOffset;

    return {
        .top    = { static_cast<float>(renderTargetX), Clamp(topY, YLow, YHigh) }, 
        .bottom = { static_cast<float>(renderTargetX), Clamp(bottomY, YLow, YHigh) },
        .depth  = depth,
        .normalizedDepth = normalizedDepth
    };
}

void RenderCameraYLine(CameraYLineData renderData, Color color, bool topEdge, bool bottomEdge)
{
    float darkness = Lerp(1, 0, renderData.normalizedDepth);

    DrawLineV(
        renderData.top, 
        renderData.bottom, 
        ColorDarken(color, renderData.normalizedDepth)
    );

    if(topEdge)
        DrawRectangle(renderData.top.x - 1, renderData.top.y - 1, 3, 3, GRAY);
    if(bottomEdge)
        DrawRectangle(renderData.bottom.x - 1, renderData.bottom.y - 1, 3, 3, GRAY);
}