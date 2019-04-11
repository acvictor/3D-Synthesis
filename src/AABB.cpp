#include "../include/AABB.h"
#include <iostream>

AABB::AABB()
{
    minx = miny = minz = 0;
    maxx = maxy = maxz = 1;
}

AABB::AABB(float x1, float x2, float y1, float y2, float z1, float z2)
{
    minx = x1;
    miny = y1;
    minz = z1;

    maxx = x2;
    maxy = y2;
    maxz = z2;
}

AABB::AABB(float x1, float x2, float z1, float z2)
{
    minx = x1;
    miny = 0;
    minz = z1;

    maxx = x2;
    maxy = 1;
    maxz = z2;
}

float AABB::Intersect(AABB b, int& axis)
{
    float diff = 0;
    if((-this->minz <= -b.maxz && -this->maxz >= -b.minz) && (this->minx <= b.maxx && this->maxx >= b.minx))
    {
        diff = abs(this->maxz - b.minz);
        std::cout << diff << " ";
        axis = 2;
        return -(diff + 1);
    }

    axis = -1;
    return 0;
}

