#pragma once

#include <raylib.h>
#include <vector>

#include "Renderer/RaycastingMath.hpp"

inline void DrawArrow(Vector2 start, Vector2 end, Color color = RED, float thickness = -1.f, float arrowTipLength = 3, float arrowTipWidth = 2)
{
    // Draw arrow line
    if(thickness <= 0)
        DrawLineV(start, end, color);
    else
        DrawLineEx(start, end, thickness, color);

    // Draw arrow tip
    Vector2 arrowDirection = Vector2Normalize(Vector2Subtract(start, end));
    Vector2 arrowRightPerpDirection = { -arrowDirection.y, arrowDirection.x };
    Vector2 arrowLeftPerpDirection = { arrowDirection.y, -arrowDirection.x };
    
    Vector2 arrowTipEnd = Vector2Add(end, Vector2Scale(arrowDirection, arrowTipLength));
    Vector2 arrowTipA = Vector2Add(arrowTipEnd, Vector2Scale(arrowRightPerpDirection, arrowTipWidth));
    Vector2 arrowTipB = Vector2Add(arrowTipEnd, Vector2Scale(arrowLeftPerpDirection, arrowTipWidth));

    DrawTriangle(end, arrowTipA, arrowTipB, color);
}