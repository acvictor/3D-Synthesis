#include "../include/Image.h"
#include "../include/Label.h"

#define cout(a) cout<<a<<endl

bool drawBox = 0;
bool show = 0;
float factor = 1.0f;

Image::Image()
{
	imgHeight = 0;
	imgWidth = 0;

    float inv[16] = {6.75744681e+00, -2.13391632e+00,  5.58003700e+05, -5.60626283e+05,
                    -3.85735891e-18, -5.92902000e-02, -1.80261253e+03,  1.82708746e+03,
                     6.43634417e-18,  6.97225412e-03,  2.51514684e+03, -2.48536116e+03,
                     1.83160963e-19, -1.72766341e-18, -4.36339962e-14,  1.00000000e+00};
    inverse = glm::make_mat4(inv);
}

template <typename T1, typename T2>
struct lessSecond {
    typedef pair<T1, T2> type;
    bool operator ()(type const& a, type const& b) const 
    {
        return a.second > b.second;
    }
};

float norm2(glm::vec2 a, glm::vec2 b)
{
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    return dx * dx + dy * dy;
}

glm::vec2 Image::Project(glm::vec3 p, glm::mat4 m)
{
    float x = m[0][0] * p.x + m[0][1] * p.y + m[0][2] * p.z + m[0][3] * 1;
    float y = m[1][0] * p.x + m[1][1] * p.y + m[1][2] * p.z + m[1][3] * 1;
    float w = m[3][0] * p.x + m[3][1] * p.y + m[3][2] * p.z + m[3][3] * 1;
    return glm::vec2(2048 * (x / w + 1) / 2.0f, 1024 - 1024 * (y / w + 1) / 2.0f);
}

float Image::Evaluate(glm::mat4 p)
{
    glm::vec2 c1 = Project(a1, p);
    glm::vec2 c2 = Project(a2, p);
    glm::vec2 c3 = Project(a3, p);
    glm::vec2 c4 = Project(a4, p);
    return norm2(p1, c1) + norm2(p2, c2) + norm2(p3, c3) + norm2(p4, c4);
}

glm::mat4 Image::Perturb()
{
    glm::mat4 p = proj;

    int i = rand() % 4;
    int j = rand() % 4;
    p[i][j] +=  (2.0f * ((float)rand() / (float)RAND_MAX) - 1.0f) / factor;

    return p;    
}

float Image::Approximate()
{
    float est = Evaluate(proj);
    
    for(int i = 0; i < 100000; i++)
    {
        glm::mat4 temp = Perturb();
        float estemp = Evaluate(temp);
        if(estemp < est)
        {
            proj = temp;
            est = estemp;
        }
    }

    return est;
}

void Image::FindProjectionMatrix(float d[4])
{
    p1 = glm::vec2(846, 528);
    p2 = glm::vec2(1101, 528);
    p3 = glm::vec2(477, 940);
    p4 = glm::vec2(1779, 940);

    a1 = glm::vec3(477, 0, d[0]);
    a2 = glm::vec3(1779, 0, d[0]);
    a3 = glm::vec3(477, 0, d[3]);
    a4 = glm::vec3(1779, 0, d[3]);

    float aaa[16] = {-0.00233721, 0.0, 0.0206022, 2.27631, 
                      1.04394e-06, 0.58, -0.0394115, 2.22219, 
                      0.0, 0.0, -1.0, 0.2, 
                     -1.27455e-06, 0.0, -0.239274, -1.03891};
    /*float aaa[16] = {0.29,  0.0,  0.0, 0.0, 
                      0.0, 0.58,  0.0, 0.0, 
                      0.0,  0.0, -1.0, 0.2, 
                      0.0,  0.0, -1.0, 0.0};*/
    proj = glm::make_mat4(aaa);

    srand(time(0));

    float err = 0.0;

    for(int i = 0; i < 1000; i++) 
    {
        factor = 100000.0f;
        err = Approximate();
        factor = 10000.0f;
        err = Approximate();
        factor = 100000.0f;
        err = Approximate();
        if(i % 100 == 0)
            cout << err << endl;
    }

    cout << err << endl;
    for(int i = 0; i < 4; i++)
    {
        for(int j = 0; j < 4; j++)
        {
            cout << proj[i][j] << " ";
        }
        cout << endl;    
    }
    cout << endl;
}

void Image::GetDepth(string fName)
{
    static float depth[2048][1024];
    ifstream file(fName);

    string im = "temp/" + name + "png";

    Mat img = imread(im, CV_LOAD_IMAGE_COLOR); 
    
    if(!img.data)                              
    {
        cout <<  "Could not open or find the image" << std::endl ;
        return;
    }

    if (file.is_open())
    {
        float v;
        for (size_t i = 0; i < imgHeight; i++)
        {
            for(size_t j = 0; j < imgWidth; j++)
            {
                file >> depth[j][i];
            }
        }

        float t[4] = {depth[846][528], depth[1101][528], depth[477][940], depth[1779][940]};
        
        /* 
         * Uncomment to compute Projection Matrix
         * 
         *
         FindProjectionMatrix(t);
         */

        // (y)i - rows, (x)j - cols, (i, j)
        #pragma omp parallel for
        for(size_t k = 0; k < segments.size(); k++)
        {
            float averageDepth = 0.0;
            Colour col = labels[segments[k].label];

            int xStart = (int)(segments[k].box.x1);
            int yStart = (int)(segments[k].box.y1);
            int xStop = (int)(segments[k].box.x2);
            int yStop = (int)(segments[k].box.y2);

            int nOfPixels = 0, red, green, blue;
            Vec3f intensity;
            #pragma omp parallel for collapse(2)
            for(int i = xStart; i < xStop; i++)
            {
                for(int j = yStart; j < yStop; j++)
                {
                    intensity = img.at<Vec3b>(j, i);
                    blue = intensity.val[0];
                    green = intensity.val[1];
                    red = intensity.val[2];

                    if(col.r == red && col.g == green && col.b == blue)
                    {
                        nOfPixels++;
                        averageDepth += depth[i][j];
                        segments[k].box.minDepth = min(segments[k].box.minDepth, depth[i][j]);
                        segments[k].box.maxDepth = max(segments[k].box.maxDepth, depth[i][j]);
                    }
                }
            }

            if(nOfPixels == 0)
            {
                cout << "Unable to get depth for " << segments[k].label << "\nObject number " << k + 1 << " in " << name << "json\n";
                nOfPixels++;                
            }

            averageDepth /= (float)nOfPixels;
            segments[k].box.averageDepth = averageDepth;
        }
    }
    else
    {
        cout << "Unable to open file\n";
        return;
    }
}

void Image::DrawSegments(string f)
{
    std::size_t found = f.find_last_of("/");
    string fName = f.substr(found + 1);

    Mat image = Mat::zeros(imgHeight, imgWidth, CV_8UC3);

    for(size_t i = 0; i < segments.size(); i++)
    {
        Point poly[1][(int)segments[i].pointCount];
        for(size_t j = 0; j < segments[i].pointCount; j++)
        {
            poly[0][j] = Point(segments[i].polygon[j].x, segments[i].polygon[j].y);
        }
        
        const Point* ppt[1] = { poly[0] };
        int npt[] = { (int)segments[i].pointCount };

        Colour col = labels[segments[i].label];

        fillPoly(image, ppt, npt, 1, Scalar(col.b, col.g, col.r), 8 );

        if(drawBox)
            rectangle(image, Point(segments[i].box.x1, segments[i].box.y1), Point(segments[i].box.x2, segments[i].box.y2), Scalar(255, 255, 255));
    }

    name = fName.substr(0, fName.length() - 4);
    imwrite("temp/" + fName.substr(0, fName.length() - 4) + "png", image);

    if(show)
    {
        imshow("Image", image);
        waitKey(0);
    }
}

void Image::PrintDepth()
{
    for(size_t i = 0; i < segments.size(); i++)
    {
        cout << segments[i].label << "\nAverge depth = " << segments[i].box.averageDepth;
        cout << "\nMin depth = " << segments[i].box.minDepth << " Max Depth" << segments[i].box.maxDepth; 

        cout << endl;
    }
}

void Image::PrintSegments()
{
    cout(imgHeight);
    cout(imgWidth);
    for(size_t i = 0; i < segments.size(); i++)
    {
        cout << segments[i].label << endl;

        for(size_t j = 0; j < segments[i].polygon.size(); j++)
        {
            cout << segments[i].polygon[j].x << " " << segments[i].polygon[j].y << endl;
        }
        cout << endl;
    }
    cout << endl;
}

bool Image::lessThan(const Segment a, const Segment b)
{
	return (a.box.y2 < b.box.y2);
}

void Image::Adjust()
{
    std::sort(segments.begin(), segments.end(), lessThan);
	
	for(size_t i = segments.size() - 1; i > 0; i--)
	{
		// Y increases downwards
		if(segments[i].box.averageDepth > segments[i - 1].box.averageDepth)
		{
			segments[i].box.averageDepth = segments[i - 1].box.averageDepth;
		}
	}	
}

void Image::InverseProject()
{
    //#pragma omp parallel for
    for(size_t i = 0; i < segments.size(); i++)
    {
        float x1 = segments[i].box.x1;
        float y1 = segments[i].box.y1;
        float x2 = segments[i].box.x2;
        float y2 = segments[i].box.y2;

        float near = .1, far = 300.0;
        float z = (1/segments[i].box.averageDepth - 1/near)/(1/far - 1/near);

        float x1dash = inverse[0][0] * x1 + inverse[0][1] * y1 + inverse[0][2] * z + inverse[0][3] * 1;
        float x2dash = inverse[0][0] * x2 + inverse[0][1] * y2 + inverse[0][2] * z + inverse[0][3] * 1;
        

        float avgChange = ((x1dash - x1) / 2.0 + (x2dash - x2) / 2.0) / 2.0;

        segments[i].box.x1 += avgChange;
        segments[i].box.x2 += avgChange; 
        
    }
}

void Image::ComputeBoundingBox()
{
    #pragma omp parallel for
    for(size_t i = 0; i < segments.size(); i++)
    {
        segments[i].ComputeBoundingBox();
    }
}

void Image::ReadJson(string fName)
{
    string line;
    ifstream file(fName);
    if (file.is_open())
    {
        while (!file.eof())
        {
            getline (file, line);
            string word = "imgHeight\": ";
            size_t found = line.find(word);
            if (found != string::npos) 
            {
                int pos = found + word.length();
                imgHeight = stoi(line.substr(pos, line.length() - pos - 1)); 
                break;
            }
        }

        while (!file.eof())
        {
            getline (file, line);
            string word = "imgWidth\": ";
            size_t found = line.find(word);
            if (found != string::npos) 
            {
                int pos = found + word.length();
                imgWidth = stoi(line.substr(pos, line.length() - pos - 1)); 
                break;
            }
        }
        while (!file.eof())
        {
            Segment newSegment;
            getline (file, line);
            string word = "label\": \"";
            size_t found = line.find(word);
            if (found != string::npos) 
            {
                int pos = found + word.length();
                newSegment.label = line.substr(pos, line.length() - pos - 3);    
                while (!file.eof())
                {
                    string start = "    [";
                    string end = "}";

                    getline (file, line);

                    if(line.find(start) != string::npos)
                    {
                        SegPoint point;

                        getline (file, line);
                        line.erase (std::remove (line.begin(), line.end(), ' '), line.end());
                        point.x = atof(line.substr(0, line.length() - 1).c_str());   

                        getline (file, line);
                        line.erase (std::remove (line.begin(), line.end(), ' '), line.end());
                        point.y = atof(line.substr(0, line.length()).c_str());   

                        newSegment.polygon.push_back(point);

                        newSegment.pointCount += 1;
                    }

                    if (line.find(end) != string::npos) 
                    {
                        break;
                    }
                }
                segments.push_back(newSegment);
            }
        }
    }
    else
    { 
        cout << "Unable to open file\n";
        return;
    }

    file.close();
}