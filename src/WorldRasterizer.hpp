#pragma once

#include <raylib.h>

#include "RaycastingMath.hpp"
#include "RaycastingCamera.hpp"
#include "ColorHelper.hpp"
#include "World.hpp"

#include <limits>
#include <stack>
#include <iostream>

struct RayCastHitInfo 
{
    float hitDistance = std::numeric_limits<float>::max();
    Vector2 hitPosition;
    const Wall* hitWall = nullptr;
};

void RenderSectorHit(const RayCastHitInfo& hitInfo, const RaycatingCamera& cam, int screenX, float floorVerticalOffset)
{
    int renderAreaWidth = cam.renderTexture.texture.width;
    int renderAreaHeight = cam.renderTexture.texture.height;

    float distance = Clamp(hitInfo.hitDistance, 0, cam.farPlaneDistance);

    // Normalize distance to [0, 1]
    float tBrightness = distance / cam.farPlaneDistance;
    uint8_t brightness = Lerp(255, 0, tBrightness);

    float rayDirectionDeg = cam.fov * (floor(0.5 * renderAreaWidth) - screenX) / renderAreaWidth;
    float rayProjectionPositionInScreen = 0.5 * tanf(rayDirectionDeg * DEG2RAD) / tanf((0.5 * cam.fov) * DEG2RAD);
    float objectHeight = round(renderAreaHeight * cam.nearPlaneDistance / (distance * cosf(rayDirectionDeg * DEG2RAD)));

    // terrain rendering
    {
        float heightDelta = renderAreaHeight - objectHeight;
        float halfHeightDelta = heightDelta / 2;

        float topY = (halfHeightDelta - floorVerticalOffset);
        float bottomY = (topY + objectHeight);

        DrawLine(
            screenX, topY, 
            screenX, bottomY, 
            ColorAlpha255(hitInfo.hitWall->color, brightness)
        );
    }
}

float RayAngleforScreenX(int screenX, float fov, int renderAreaWidth)
{
    float fovRate = fov / renderAreaWidth;
    float angle = -(fov / 2);

    return angle + (fovRate * screenX);
}

RayCastHitInfo GetHitInfosForSectorAtScreenX(const Sector& sector, World& world, const RaycatingCamera& cam, int screenX, float floorVerticalOffset, uint32_t sourceSectorId)
{
    RayCastHitInfo hitInfo;

    float rayAngle = RayAngleforScreenX(screenX, cam.fov, cam.renderTexture.texture.width);

    RasterRay ray = {
        .position = cam.position,
        .direction = Vector2DirectionFromAngle((rayAngle * DEG2RAD) + cam.yaw),
    };

    for(const Wall& wall : sector.segments)
    {
        HitInfo hit;

        if(RayToSegmentCollision(ray, wall.segment, hit))
        {
            float distance = Vector2Distance(ray.position, hit.position);

            if (NULL_SECTOR != wall.windowToSector)
            {
                if (wall.windowToSector != sourceSectorId)
                {
                    if (world.sectors.contains(wall.windowToSector))
                    {
                        const Sector& nextSector = world.sectors.at(wall.windowToSector);

                        RayCastHitInfo nextHitInfo =
                            GetHitInfosForSectorAtScreenX(nextSector, world, cam, screenX, floorVerticalOffset, sourceSectorId);

                        if (nextHitInfo.hitDistance < hitInfo.hitDistance)
                        {
                            hitInfo = nextHitInfo;
                        }
                    }
                    else
                    {
                        std::cerr << "Invalid window to sector: " << wall.windowToSector << std::endl;
                    }
                }
                else
                {
                    // Skip
                }
            }
            else if (distance < hitInfo.hitDistance)
            {
                hitInfo.hitDistance = distance;
                hitInfo.hitPosition = hit.position;
                hitInfo.hitWall = &wall;
            }
        }
    }

    return hitInfo;
}

void RasterizeWorld(World& world, const RaycatingCamera& cam)
{
    BeginTextureMode(cam.renderTexture);
    {
        ClearBackground(MY_BLACK);

        int renderAreaWidth = cam.renderTexture.texture.width;
        int renderAreaHeight = cam.renderTexture.texture.height;

        float floorVerticalOffset = round(0.5f * renderAreaHeight * (tanf(cam.pitch)) / tanf(0.5f * cam.fovVectical));

        // floor / ceiling rendering
        // {
        //     float center = (renderAreaHeight / 2) - floorVerticalOffset;
        //     DrawRectangle(0, 0, renderAreaWidth, center, BLUE);
        //     DrawRectangle(0, center, renderAreaWidth, renderAreaHeight, GRAY);
        // }

        // Terrain rendering
        {
            Sector& sector = world.sectors.at(cam.currentSector);

            for(int screenX = 0; screenX < renderAreaWidth; ++screenX)
            {
                RayCastHitInfo hitInfo = GetHitInfosForSectorAtScreenX(sector, world, cam, screenX, floorVerticalOffset, sector.id);

                if (hitInfo.hitDistance < std::numeric_limits<float>::max())
                {
                    RenderSectorHit(hitInfo, cam, screenX, floorVerticalOffset);
                }
                else
                {
                    //DrawLine(screenX, 0, screenX, renderAreaHeight, PURPLE);
                }
            }
        }
    }
    EndTextureMode();
}

// struct ScreenSlice { int x0; int x1; };

// struct RenderingContext
// {
//     const Sector& sector;
//     const ScreenSlice screenSlice;
// };

// std::stack<RenderingContext> sectorsRenderingQueue;

// sectorsRenderingQueue.push(
//     {
//         .sector = world.sectors.at(cam.currentSector),
//         .screenSlice = { 0, renderAreaWidth }
//     }
// );

// if(wall.windowToSector > 0)
// {
//     const Sector& nextSector = world.sectors.at(wall.windowToSector);
    
//     for(const Wall& wall : nextSector.segments)
//     {
//         HitInfo hit;

//         if(wall.windowToSector > 0)
//         {
//             // Skip
//         }
//         else if(RayToSegmentCollision(ray, wall.segment, hit))
//         {
//             float distance = Vector2Distance(ray.position, hit.position);

//             if(distance < hitInfo.hitDistance)
//             {
//                 hitInfo.hitDistance = distance;
//                 hitInfo.hitPosition = hit.position;
//                 hitInfo.hitWall = &wall;
//             }
//         }
//     }
// }
// else 


// while(0 != sectorsRenderingQueue.size())
// {
//     RenderingContext renderCtx = sectorsRenderingQueue.top();
//     sectorsRenderingQueue.pop();
    
//     for(int screenX = renderCtx.screenSlice.x0; screenX < renderCtx.screenSlice.x1; ++screenX)
//     {
//         float rayAngle = RayAngleforScreenX(screenX, cam.fov);

//         RasterRay ray = {
//             .position = cam.position,
//             .direction = Vector2DirectionFromAngle((rayAngle * DEG2RAD) + cam.yaw),
//         };
        
//         RayCastingHitInfo hitInfo;

//         for(const Wall& wall : renderCtx.sector.segments)
//         {
//             HitInfo hit;

//             if(RayToSegmentCollision(ray, wall.segment, hit))
//             {
//                 float distance = Vector2Distance(ray.position, hit.position);

                
//                 if(distance < hitInfo.hitDistance)
//                 {
//                     hitInfo.hitDistance = distance;
//                     hitInfo.hitPosition = hit.position;
//                     hitInfo.hitWall = &wall;
//                 }
//             }
//         }

//         if(hitInfo.hitDistance < std::numeric_limits<float>::max())
//         {
//             RenderSectorHit(hitInfo, cam, screenX, floorVerticalOffset);
//         }
//         else
//         {
//             DrawLine(screenX, 0, screenX, renderAreaHeight, PURPLE);
//         }
//     }
// }