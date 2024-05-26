#include "WorldRasterizer.hpp"

#include "RaycastingMath.hpp"
#include "ColorHelper.hpp"

#include <limits>
#include <iostream>
#include <unordered_map>

void RasterizeWorld(const World& world, const RaycastingCamera& cam)
{
    BeginTextureMode(cam.renderTexture);
    {
        ClearBackground(MY_BLACK);

        const uint32_t RenderTargetWidth = cam.renderTexture.texture.width;
        const uint32_t RenderTargetHeight = cam.renderTexture.texture.height;

        // Dummy sky / ground
        // {
        //     float floorVerticalOffset = round(0.5f * RenderTargetHeight * (tanf(cam.pitch)) / tanf(0.5f * cam.fovVectical));

        //     float center = (RenderTargetHeight / 2) - floorVerticalOffset;
        //     DrawRectangle(0, 0, RenderTargetWidth, center, BLUE);
        //     DrawRectangle(0, center, RenderTargetWidth, RenderTargetHeight, GRAY);
        // }


        RenderAreaYBoundaries yBoundaries;
        yBoundaries.resize(RenderTargetWidth);
        
        // Init bounds
        for (auto& bound : yBoundaries) 
        {
            bound.yHigh = cam.renderTexture.texture.height;
            bound.yLow  = 0;
        }

        std::stack<SectorRenderContext> renderStack;
        
        renderStack.push({
            .world = &world,
            .sector = &world.Sectors.at(cam.currentSector),
            .renderArea = {
                .xBegin = 0,
                .xEnd = RenderTargetWidth - 1,
                .yBoundaries = &yBoundaries
            },
        });

        size_t maxRenderItr = 25;

        for(size_t i = 0; 
            0 < renderStack.size() && i < maxRenderItr;
            ++i
        )
        {
            RasterizeInRenderArea(renderStack.top(), renderStack, cam);
        }
    }

    EndTextureMode();
}

void RasterizeInRenderArea(SectorRenderContext renderContext, std::stack<SectorRenderContext>& renderStack, const RaycastingCamera& cam)
{
    std::unordered_map<const Wall*, SectorRenderContext> renderAreaToPushInStack;

    auto& [ world, sector, renderArea ] = renderContext;

    int renderAreaHeight = cam.renderTexture.texture.height;

    // to avoid multiple compute
    float floorVerticalOffset = round(0.5f * renderAreaHeight * (tanf(cam.pitch)) / tanf(0.5f * cam.fovVectical));

    for(uint32_t x = renderArea.xBegin; x <= renderArea.xEnd; ++x)
    {
        RenderAreaYBoundary& xBounds = renderArea.yBoundaries->at(x);

        float rayAngle = RayAngleforScreenXCam(x, cam);

        RasterRay rasterRay = {
            .position = cam.position,
            .direction = Vector2DirectionFromAngle((rayAngle * DEG2RAD) + cam.yaw),
        };

        RaycastHitData bestHitData;

        for(const Wall& wall : sector->walls)
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
                float centerY = (cam.renderTexture.texture.height / 2) - floorVerticalOffset;
                DrawLine(x, xBounds.yLow, x, centerY, sector->ceilingColor);
                // Draw Ceiling
                DrawLine(x, centerY, x, xBounds.yHigh, sector->floorColor);
            }
            
            // Means this is a slid wall
            if(bestHitData.wall->toSector == NULL_SECTOR)
            {
                CameraYAxisData cameraWallYData = 
                    ComputeCameraYAxis(cam, x, bestHitData.distance, floorVerticalOffset,
                        xBounds.yHigh,
                        xBounds.yLow,
                        sector->zFloor, sector->zCeiling
                    );

                RenderCameraYAxis(cameraWallYData, bestHitData.wall->color);
            }
            else
            {
                if(!world->Sectors.contains(bestHitData.wall->toSector))
                {
                    std::cerr << "Try to render a window with an invalid SectorID (" << bestHitData.wall->toSector << ")"<< std::endl;
                    continue;
                }

                const Sector& nextSector = world->Sectors.at(bestHitData.wall->toSector);

                //  Render borders acording to sector zceiling & zflor



                /* --------------- Check if not fixable with yLow yHigh fix ---------------- */

                const Sector* refrenceSector = &nextSector;
                bool extraBorder = false;

                if(nextSector.zCeiling > sector->zCeiling)
                {
                    refrenceSector = sector;
                    extraBorder = false;
                }
                else
                {
                    extraBorder = true;
                }

                auto topBorderRenderData = ComputeCameraYAxis(cam, x, bestHitData.distance, floorVerticalOffset, 
                    xBounds.yHigh,
                    xBounds.yLow,
                    0, refrenceSector->zCeiling
                );    
                RenderCameraYAxis(topBorderRenderData, refrenceSector->ceilingColor, extraBorder, true);

                if(nextSector.zFloor > sector->zFloor)
                {
                    refrenceSector = sector;
                    extraBorder = false;
                }
                else
                {
                    extraBorder = true;
                }
                
                auto bottomBorderRenderData = ComputeCameraYAxis(cam, x, bestHitData.distance, floorVerticalOffset, 
                    xBounds.yHigh,
                    xBounds.yLow,
                    refrenceSector->zFloor, 0
                );
                RenderCameraYAxis(bottomBorderRenderData, refrenceSector->floorColor, true, extraBorder);
                
                /* --------------- [END] Check if not fixable with yLow yHigh fix ---------------- */



                // Apply Y boundaries

                if(bottomBorderRenderData.top.y < xBounds.yHigh)
                    xBounds.yHigh = bottomBorderRenderData.top.y;
                if(bottomBorderRenderData.bottom.y > xBounds.yLow)
                    xBounds.yLow = topBorderRenderData.bottom.y;

                // Draw a Purple
                DrawLineV({(float)x, (float)xBounds.yLow}, { (float)x, (float)xBounds.yHigh }, PURPLE);

                // Create / update NextRenderArea

                if(!renderAreaToPushInStack.contains(bestHitData.wall))
                {
                    SectorRenderContext nextRenderAreaContext = {
                        .world = world,
                        .sector = &nextSector,
                        .renderArea = {
                            .xBegin = x,
                            .xEnd = x,
                            .yBoundaries = renderArea.yBoundaries
                        }
                    };

                    renderAreaToPushInStack.emplace(bestHitData.wall, nextRenderAreaContext);
                }
                else
                {
                    renderAreaToPushInStack[bestHitData.wall].renderArea.xEnd = x;
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

CameraYAxisData ComputeCameraYAxis(
    const RaycastingCamera& cam, uint32_t renderTargetX, float hitDistance, float floorVerticalOffset,
    uint32_t YHigh, uint32_t YLow,
    float topOffsetPercentage, float bottomOffsetPercentage
)
{
    const int RenderTargetWidth = cam.renderTexture.texture.width;
    const int RenderTargetHeight = cam.renderTexture.texture.height;

    const float depth = Clamp(hitDistance, 0, cam.farPlaneDistance);
    // Normalize distance to [0, 1]
    const float normalizedDepth = depth / cam.farPlaneDistance;

    const float rayDirectionDeg = cam.fov * (floor(0.5 * RenderTargetWidth) - renderTargetX) / RenderTargetWidth;
    const float rayProjectionPositionInScreen = 0.5 * tanf(rayDirectionDeg * DEG2RAD) / tanf((0.5 * cam.fov) * DEG2RAD);
    const float objectHeight = round(RenderTargetHeight * cam.nearPlaneDistance / (depth * cosf(rayDirectionDeg * DEG2RAD)));

    // Rendering
    const float heightDelta = RenderTargetHeight - objectHeight;
    const float halfHeightDelta = heightDelta / 2;
    
    const float fullSizeTopY = (halfHeightDelta - floorVerticalOffset);
    
    float topY    = fullSizeTopY + (objectHeight * topOffsetPercentage);
    float bottomY = (fullSizeTopY + objectHeight) - (objectHeight * bottomOffsetPercentage);

    return {
        .top    = { static_cast<float>(renderTargetX), Clamp(topY, YLow, YHigh) }, 
        .bottom = { static_cast<float>(renderTargetX), Clamp(bottomY, YLow, YHigh) },
        .depth  = depth,
        .normalizedDepth = normalizedDepth
    };
}

void RenderCameraYAxis(CameraYAxisData renderData, Color color, bool topBorder, bool bottomBorder)
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