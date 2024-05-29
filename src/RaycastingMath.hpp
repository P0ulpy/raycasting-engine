#pragma once

#include <raylib.h>
#include <raymath.h>

#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <cstdint>

#include "ColorHelper.hpp"

struct Vector2i
{
    int x { 0 };
    int y { 0 };
};

struct Vector2ui
{
    unsigned int x { 0 };
    unsigned int y { 0 };
};

inline Vector2 Vector2DirectionFromAngle(float angleRadian, float length = 1)
{
    return {
        length * cosf(angleRadian),
        length * sinf(angleRadian),
    };
}

inline float Vector2DirectionToAngle(Vector2 direction)
{
    return atan2f(direction.y, direction.x);
}
struct Segment
{
    Vector2 a { 0 };
    Vector2 b { 0 };
};

using SectorID = uint32_t;

constexpr SectorID NULL_SECTOR { static_cast<SectorID>(-1) };
struct Wall
{
    Segment segment;
    SectorID toSector = NULL_SECTOR;
    Color color = WHITE;
};

struct Sector
{
    std::vector<Wall> walls;
    Color floorColor = MY_DARK_BLUE;
    Color ceilingColor = MY_BEIGE;
    Color topBorderColor = MY_PURPLE;
    Color bottomBorderColor = MY_RED;
    float zCeiling = 1;
    float zFloor = 1;
};

struct RasterRay
{
    Vector2 position { 0 };
    Vector2 direction { 0 };

    float Yaw()
    {
        return Vector2DirectionToAngle(direction);
    }

    void LookAt(const Vector2& positionLook)
    {
        direction = Vector2Subtract(position, positionLook);
        direction = Vector2Normalize(direction);
    }
};

struct HitInfo
{
    Vector2 position { 0 };
    float distance = 0;
};

inline bool RayToSegmentCollision(const RasterRay& ray, const Segment& seg, HitInfo& hitInfo)
{
    const float x1 = seg.a.x;
    const float y1 = seg.a.y;
    const float x2 = seg.b.x;
    const float y2 = seg.b.y;
    
    const float x3 = ray.position.x;
    const float y3 = ray.position.y;
    const float x4 = ray.position.x + ray.direction.x;
    const float y4 = ray.position.y + ray.direction.y;

    const float devider = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);

    // the tow segements are perfectly parallels
    if(devider == 0) return false;

    const float t = ((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)) / devider;
    const float u = ((x1 - x3) * (y1 - y2) - (y1 - y3) * (x1 - x2)) / devider;
    
    // old wikipedia formula for u
    //const u = -((x1 - x2) * (y1 - y3) - (y1 - y2) * (x1 - x3)) / devider;
    
    if(t > 0 && t < 1 && u > 0)
    {
        const float xCollision = x1 + t * (x2 - x1);
        const float yCollision = y1 + t * (y2 - y1);

        const Vector2 hitPosition = { xCollision, yCollision };
        
        hitInfo  = {
            .position = hitPosition,
            .distance = Vector2Distance(ray.position, hitPosition)
        };

        return true;
    }

    return false;
}

inline float RayAngleforScreenX(int screenX, float fov, int renderAreaWidth)
{
    float fovRate = fov / renderAreaWidth;
    float angle = -(fov / 2);

    return angle + (fovRate * screenX);
}

// < 0 = right, 0 = on, > 0 = left
inline constexpr float PointSegmentSide(Vector2 point, Vector2 a, Vector2 b)
{
    return -(((point.x - a.x) * (b.y - a.y)) - ((point.y - a.y) * (b.x - a.x)));
}

inline Vector2 InsidePoint(const std::vector<Wall>& walls)
{
    Vector2 insidePoint = std::accumulate(walls.begin(), walls.end(), Vector2(0, 0), 
        [](const Vector2& a, const Wall& b) { return Vector2Add(a, b.segment.a); });

    float wallsSize = static_cast<float>(walls.size());
    insidePoint.x /= wallsSize;
    insidePoint.y /= wallsSize;

    return insidePoint;
}

inline bool IsPointInSector(Vector2 point, const Sector& sector) 
{
    std::vector<Wall> walls(sector.walls);

    // Find a point inside the sector
    Vector2 insidePoint = InsidePoint(walls);

    // Order the wall segments based on the angle they make with the inside point
    std::sort(walls.begin(), walls.end(), 
        [insidePoint](const Wall& a, const Wall& b) 
        {
            float angleA = std::atan2(a.segment.a.y - insidePoint.y, a.segment.a.x - insidePoint.x);
            float angleB = std::atan2(b.segment.a.y - insidePoint.y, b.segment.a.x - insidePoint.x);
            return angleA < angleB;
        });

    // Use the ray casting algorithm
    bool inside = false;
    for (size_t i = 0, j = walls.size() - 1; i < walls.size(); j = i++) 
    {
        Vector2 a = walls[i].segment.a;
        Vector2 b = walls[j].segment.b;
        if ((a.y > point.y) != (b.y > point.y) &&
            (point.x < (b.x - a.x) * (point.y - a.y) / (b.y - a.y) + a.x))
        {
            inside = !inside;
        }
    }

    return inside;
}
