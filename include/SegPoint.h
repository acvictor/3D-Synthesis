#pragma once

class SegPoint
{
public:
	float x, y;

	SegPoint()
	{
		x = 0.0;
		y = 0.0;
	}

	SegPoint(float x_, float y_)
	{
		x = x_;
		y = y_;
	}
};