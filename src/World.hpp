#pragma once

#include "RaycastingMath.hpp"

#include <vector>
#include <unordered_map>

constexpr uint32_t NULL_SECTOR { static_cast<uint32_t>(-1) };

struct Wall
{
    Segment segment;
    uint32_t toSector = NULL_SECTOR;
    Color color = WHITE;
};

struct Sector
{
    std::vector<Wall> walls;
    float zfloor = 0;
    float zceiling = 0;
};

struct World
{
    std::unordered_map<uint32_t, Sector> Sectors = 
    {
        {
            1, 
            {
                .walls = {
                    {
                        .segment = { { 500, 500 }, { 500, 700 } }, 
                        .color = RED,
                    },
                    { 
                        .segment = { { 500, 700 }, { 700, 700 } }, 
                        .color = BLUE,
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
                        .color = GREEN,
                    },
                    {
                        .segment = { { 700, 500 }, { 500, 500 } }, 
                        .color = YELLOW,
                    },
                    // {
                    //     .segment = { { 500, 500 }, { 700, 700 } },
                    //     .toSector = 1U,
                    //     .color = PURPLE,
                    // },
                }
            },
        },
    };
};