#pragma once

class AABB
{
public:
    float minx, miny, maxx, maxy, minz, maxz;

    AABB();
    AABB(float, float, float, float, float, float);
    AABB(float, float, float, float);

    float Intersect(AABB a, int& axis);
};