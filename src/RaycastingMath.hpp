#pragma once

#include <raylib.h>
#include <raymath.h>
#include <cmath>
#include <cstdint>

Vector2 Vector2DirectionFromAngle(float angleRadian, float length = 1)
{
    return {
        length * cosf(angleRadian),
        length * sinf(angleRadian),
    };
}

float Vector2DirectionToAngle(Vector2 direction)
{
    return atan2f(direction.y, direction.x);
}

struct Segment
{
    Vector2 a { 0 };
    Vector2 b { 0 };
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
};

bool RayToSegmentCollision(const RasterRay& ray, const Segment& seg, HitInfo& hitInfo)
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
            .position = hitPosition
        };

        return true;
    }

    return false;
}
