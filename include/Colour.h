#pragma once

class Colour
{
public:
	int r, g, b;

	Colour(int r_, int g_, int b_)
	{
		r = r_;
		g = g_;
		b = b_;
	}

	Colour() 
	{
		r = 0;
		g = 0;
		b = 0;
	}
};