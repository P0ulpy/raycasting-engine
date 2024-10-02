#include "World.hpp"

World::World()
{
    InitWorld();
}

void World::InitWorld()
{
    for(auto& [ sectorId, sector ] : Sectors)
    {
        RearrangeWallListToPolygon(sector.walls);
    }
}

void RearrangeWallListToPolygon(std::vector<Wall> &walls)
{
    // Find a point inside the sector
    Vector2 insidePoint = FindInsidePoint(walls);

    // Order the wall segments based on the angle they make with the inside point
    std::sort(walls.begin(), walls.end(), 
        [insidePoint](const Wall& a, const Wall& b) 
        {
            float angleA = std::atan2(a.segment.a.y - insidePoint.y, a.segment.a.x - insidePoint.x);
            float angleB = std::atan2(b.segment.a.y - insidePoint.y, b.segment.a.x - insidePoint.x);
            return angleA < angleB;
        });
}

uint32_t FindSectorOfPoint(Vector2 point, const World &world)
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