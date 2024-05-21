#pragma once

#include "RaycastingMath.hpp"
#include <vector>
#include <unordered_map>

constexpr uint32_t NULL_SECTOR = 0;

struct Wall
{
    Segment segment;
    Color color = WHITE;
    uint32_t windowToSector = NULL_SECTOR;
};

struct Sector
{
    uint32_t id;
    std::vector<Wall> segments;
    float zfloor = 0;
    float zceiling = 0;
};

struct World
{
    std::unordered_map<uint32_t, Sector> sectors =
    {
        { 1, {
                .id = 1,
                .segments = {
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
                        .color = PURPLE,
                        .windowToSector = 2,
                    },
                }
            },
        },
        { 2, {
                .id = 2,
                .segments = {
                    { 
                        .segment = { { 500, 500 }, { 700, 700 } }, 
                        .color = PURPLE,
                        .windowToSector = 1,
                    },
                    { 
                        .segment = { { 700, 700 }, { 700, 500 } }, 
                        .color = GREEN,
                    },
                    { 
                        .segment = { { 700, 500 }, { 500, 500 } }, 
                        .color = YELLOW,
                    },
                },
            },
        },
    };
};