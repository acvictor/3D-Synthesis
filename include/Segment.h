#pragma once

#include <bits/stdc++.h>

#include "SegPoint.h"
#include "BoundingBox.h"

#define inf 10000

class Segment
{
public:
	std::vector<SegPoint> polygon;
	std::string label;
	int pointCount;
	BoundingBox box;

	Segment();

	void ComputeBoundingBox(); 
};