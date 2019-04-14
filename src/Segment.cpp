#include "../include/Segment.h"

Segment::Segment()
{
	label = "";
	pointCount = 0;
	box = BoundingBox();
}

void Segment::ComputeBoundingBox(std::string label)
{
	float minX = inf, minY = inf; 
	float maxX = -inf, maxY = -inf;

	/*if(label == "sidewalk")
	{
		int ind1, ind2;
		for(size_t i = 0; i < polygon.size(); i++)
		{
			if(maxY < polygon[i].y)
			{
				maxY = polygon[i].y;
				ind1 = i;
			}

			if(minY > polygon[i].y)
			{
				minY = polygon[i].y;
				ind2 = i;
			}
		}

		box = BoundingBox(polygon[ind2].x, polygon[ind2].y, polygon[ind1].x, polygon[ind1].y);
		// std::cout << polygon[ind2].x << " " << polygon[ind2].y << " " << polygon[ind1].x << " " << polygon[ind1].y << std::endl;
		return;
	}*/

	for(size_t i = 0; i < polygon.size(); i++)
	{
		minX = std::min(minX, polygon[i].x);
		minY = std::min(minY, polygon[i].y);

		maxX = std::max(maxX, polygon[i].x);
		maxY = std::max(maxY, polygon[i].y);
	}

	box = BoundingBox(minX, minY, maxX, maxY);	
}