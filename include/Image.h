#pragma once

#include <bits/stdc++.h>

#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <omp.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Segment.h"

using namespace std;
using namespace cv;

class Image
{
public:
	vector<Segment> segments;
    int imgHeight, imgWidth;
    string name;
    glm::mat4 proj, inverse;
    glm::vec2 p1, p2, p3, p4;
    glm::vec3 a1, a2, a3, a4;

    Image();

    void InverseProject();
    void Adjust();
    static bool lessThan(const Segment a, const Segment b);
    void ReadJson(string fName);
    void PrintSegments();
    void ComputeBoundingBox();
    void DrawSegments(string fName);
    void FindProjectionMatrix(float d[4]);
    void GetDepth(string fName);
    void PrintDepth();
};