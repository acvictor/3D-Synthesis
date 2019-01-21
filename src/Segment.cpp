#include "../include/Segment.h"

Segment::Segment()
{
	label = "";
	pointCount = 0;
	box = BoundingBox();
}

void Segment::ComputeBoundingBox()
{
	float minX = inf, minY = inf; 
	float maxX = -inf, maxY = -inf;

	for(size_t i = 0; i < polygon.size(); i++)
	{
		minX = std::min(minX, polygon[i].x);
		minY = std::min(minY, polygon[i].y);

		maxX = std::max(maxX, polygon[i].x);
		maxY = std::max(maxY, polygon[i].y);
	}

	box = BoundingBox(minX, minY, maxX, maxY);	
}