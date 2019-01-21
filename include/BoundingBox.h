#pragma once

class BoundingBox
{
public:
	/* 
	 * (x1, y1) top left
	 * (x2, y2) bottom right
	 */
	float x1, y1, x2, y2;

	BoundingBox()
	{
		x1 = 0; y1 = 0;
		x2 = 0; y2 = 0;
	}

	BoundingBox(float x1_, float y1_, float x2_, float y2_)
	{
		x1 = x1_; y1 = y1_;
		x2 = x2_; y2 = y2_;
	}
};