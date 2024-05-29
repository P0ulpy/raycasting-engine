#pragma once

#include "RaycastingMath.hpp"

#include <unordered_map>

struct World
{
    std::unordered_map<SectorID, Sector> Sectors = 
    {
        {
            3,
            {
                .walls = {
                    {
                        .segment = { { 500, 600 }, { 500, 700 } }, 
                        .toSector = 1U,
                        .color = PURPLE,
                    },
                    {
                        .segment = { { 500, 600 }, { 300, 600 } }, 
                        .color = WHITE,
                    },
                    {
                        .segment = { { 300, 600 }, { 300, 700 } }, 
                        .color = WHITE,
                    },
                    {
                        .segment = { { 300, 700 }, { 500, 700 } }, 
                        .color = WHITE,
                    },
                },
                .zFloor = 0.95
            }
        },
        {
            1, 
            {
                .walls = {
                    {
                        .segment = { { 500, 500 }, { 500, 600 } }, 
                        .color = WHITE,
                    },
                    {
                        .segment = { { 500, 700 }, { 500, 600 } }, 
                        .toSector = 3U,
                        .color = PURPLE,
                    },
                    { 
                        .segment = { { 500, 700 }, { 700, 700 } }, 
                        .color = WHITE,
                    },
                    {
                        .segment = { { 500, 500 }, { 700, 700 } },
                        .toSector = 2U,
                        .color = PURPLE,
                    },
                }
            },
        },
        { 
            2, 
            {
                .walls = {
                    { 
                        .segment = { { 700, 700 }, { 700, 500 } }, 
                        .color = WHITE,
                    },
                    {
                        .segment = { { 700, 500 }, { 500, 500 } }, 
                        .color = WHITE,
                    },
                    {
                        .segment = { { 700, 700 }, { 500, 500 } },
                        .toSector = 1U,
                        .color = PURPLE,
                    },
                },
                .floorColor = BLUE,
                .ceilingColor = RED,
                .zCeiling = 0.60f,
                .zFloor = 0.75f,
            },
        },
    };
};

inline uint32_t FindSectorOfPoint(Vector2 point, const World& world)
{
    for(const auto& [ sectorId, sector ] : world.Sectors)
    {
        if(IsPointInSector(point, sector))
        {
            return sectorId;
        }
    }

    return NULL_SECTOR;
}