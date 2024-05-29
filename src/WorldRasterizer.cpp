#include "WorldRasterizer.hpp"

#include "RaycastingMath.hpp"
#include "ColorHelper.hpp"

#include <limits>
#include <iostream>
#include <unordered_map>

RasterizeWorldContext InitRasterizeWorldContext(uint32_t RenderTargetWidth, uint32_t RenderTargetHeight, const World& world, const RaycastingCamera& cam)
{
    float FloorVerticalOffset = round(0.5f * RenderTargetHeight * (tanf(cam.pitch)) / tanf(0.5f * cam.fovVectical));

    RasterizeWorldContext context = {
        .world = world,
        .cam = cam,
        .RenderTargetWidth = RenderTargetWidth,
        .RenderTargetHeight = RenderTargetHeight,
        .FloorVerticalOffset = FloorVerticalOffset,
    };

    context.yBoundaries.resize(RenderTargetWidth);
    
    for (auto& bound : context.yBoundaries) 
    {
        bound.max = RenderTargetHeight;
        bound.min  = 0;
    }
    
    assert(world.Sectors.contains(cam.currentSector), "Try to InitRasterizeWorldContext with an invalid SectorID");

    context.renderStack.push({
        .sectorId = cam.currentSector,
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
        RasterizeInRenderArea(rasterizeContext, rasterizeContext.renderStack.top());
    }
}

void RasterizeInRenderArea(RasterizeWorldContext& worldContext, SectorRenderContext renderContext)
{
    std::unordered_map<SectorID, SectorRenderContext> renderAreaToPushInStack;

    auto& [ world, cam, RenderTargetWidth, RenderTargetHeight, FloorVerticalOffset, yBoundaries, renderStack ] = worldContext;
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
            {
                // Draw floor
                float centerY = (RenderTargetHeight / 2) - FloorVerticalOffset;
                DrawLine(x, yMinMax.min, x, centerY, currentSector.ceilingColor);
                // Draw Ceiling
                DrawLine(x, centerY, x, yMinMax.max, currentSector.floorColor);
            }
            
            // Means this is a slid wall
            if(bestHitData.wall->toSector == NULL_SECTOR)
            {
                CameraYLineData cameraWallYData = 
                    ComputeCameraYAxis(cam, x, bestHitData.distance, FloorVerticalOffset,
                        RenderTargetWidth, RenderTargetHeight,
                        yMinMax.max,
                        yMinMax.min,
                        currentSector.zFloor, currentSector.zCeiling
                    );

                RenderCameraYLine(cameraWallYData, bestHitData.wall->color);
            }
            else
            {
                if(!world.Sectors.contains(bestHitData.wall->toSector))
                {
                    std::cerr << "Try to render a next sector with an invalid SectorID (" << bestHitData.wall->toSector << ")"<< std::endl;
                    continue;
                }

                const SectorID nextSectorId = bestHitData.wall->toSector;
                const Sector& nextSector = world.Sectors.at(nextSectorId);
                
                RenderNextRenderAreaBorders(worldContext, yMinMax, currentSector, nextSector, x, bestHitData.distance);

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
                    renderAreaToPushInStack[nextSectorId].renderArea.xEnd = x;
                }
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

void RenderNextRenderAreaBorders(RasterizeWorldContext& worldContext, RenderAreaYMinMax& yMinMax, const Sector& currentSector, const Sector& nextSector, uint32_t x, float hitDistance)
{
    {
        bool nextSectCelingHigher = nextSector.zCeiling >= currentSector.zCeiling;

        const Sector& zSizesSector = (nextSectCelingHigher) ? currentSector : nextSector;
        const Color upperBorderColor = (nextSectCelingHigher) ? currentSector.ceilingColor : nextSector.topBorderColor;
        bool topBorder = !nextSectCelingHigher;

        CameraYLineData topBorderLineData = ComputeCameraYAxis(worldContext.cam, x, hitDistance, worldContext.FloorVerticalOffset, 
            worldContext.RenderTargetWidth, worldContext.RenderTargetHeight,
            yMinMax.max, yMinMax.min,
            0, zSizesSector.zCeiling
        );
        RenderCameraYLine(topBorderLineData, upperBorderColor, topBorder, true);

        // Apply Y min
        yMinMax.min = topBorderLineData.bottom.y;
    }

    {
        bool nextSectFloorHigher = nextSector.zFloor >= currentSector.zFloor;

        const Sector& zSizesSector = (nextSectFloorHigher) ? currentSector : nextSector;
        const Color lowerBorderColor = (nextSectFloorHigher) ? currentSector.floorColor : nextSector.bottomBorderColor;
        bool bottomBorder = !nextSectFloorHigher;

        CameraYLineData bottomBorderLineData = ComputeCameraYAxis(worldContext.cam, x, hitDistance, worldContext.FloorVerticalOffset, 
            worldContext.RenderTargetWidth, worldContext.RenderTargetHeight,
            yMinMax.max, yMinMax.min,
            zSizesSector.zFloor, 0
        );
        RenderCameraYLine(bottomBorderLineData, lowerBorderColor, true, bottomBorder);

        // Apply Y max
        yMinMax.max = bottomBorderLineData.top.y;
    }

    // Draw a Purple placeholder where next sector will be drawn
    DrawLineV({(float)x, (float)yMinMax.min}, { (float)x, (float)yMinMax.max }, PURPLE);
}

CameraYLineData ComputeCameraYAxis(
    const RaycastingCamera& cam, uint32_t renderTargetX, float hitDistance, float FloorVerticalOffset,
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

    return {
        .top    = { static_cast<float>(renderTargetX), Clamp(topY, YLow, YHigh) }, 
        .bottom = { static_cast<float>(renderTargetX), Clamp(bottomY, YLow, YHigh) },
        .depth  = depth,
        .normalizedDepth = normalizedDepth
    };
}

void RenderCameraYLine(CameraYLineData renderData, Color color, bool topBorder, bool bottomBorder)
{
    uint8_t brightness = Lerp(255, 0, renderData.normalizedDepth);
    
    DrawLineV(
        renderData.top, 
        renderData.bottom, 
        // ColorAlpha255(color, brightness)
        color
    );

    if(topBorder)
        DrawRectangle(renderData.top.x - 1, renderData.top.y - 1, 2, 2, GRAY);
    if(bottomBorder)
        DrawRectangle(renderData.bottom.x - 1, renderData.bottom.y - 1, 2, 2, GRAY);
}