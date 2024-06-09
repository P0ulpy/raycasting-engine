#pragma once

#include <raylib.h>

#include <vector>

#include "RaycastingMath.hpp"

void DrawSectorAsPolygon(const Sector& sector, Color color);
// {
//     if (sector.walls.size() < 3) 
//     {
//         return; // Not a polygon
//     }

//     Vector2 firstPoint = sector.walls[0].segment.a;

//     for (size_t i = 1; i < sector.walls.size() - 1; ++i) 
//     {
//         Vector2 secondPoint = sector.walls[i].segment.a;
//         Vector2 thirdPoint = sector.walls[i + 1].segment.a;

//         DrawTriangle(firstPoint, secondPoint, thirdPoint, color);
//     }
// }