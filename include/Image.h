#pragma once

#include <bits/stdc++.h>

#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "Segment.h"

using namespace std;
using namespace cv;

class Image
{
public:
	vector<Segment> segments;
    int imgHeight, imgWidth;

    Image();

    void ReadJson(string fName);
    void PrintSegments();
    void ComputeBoundingBox();
    void DrawSegments(string fName);
};