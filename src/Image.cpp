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

    /*float inv[16] = {6.15810664e+00, -5.62615291e+01, -1.12439084e+06,  1.14132474e+06,
                       6.06108404e-03, -1.57862278e-03,  1.14818895e+03, -1.14726928e+03,
                       1.13386747e-03,  4.35498197e-01,  1.54060612e+04, -1.55298191e+04,
                      -6.57158767e-19,  1.79491369e-17,  6.20669260e-13,  1.00000000e+00};*/
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

        // (y)i - rows, (x)j - cols, (i, j)
        for(size_t k = 0; k < segments.size(); k++)
        {
            float averageDepth = 0.0;
            if(segments[k].label == "sidewalk")
                continue;
                
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
                cout << segments[k].label << " not found\n";
                segments.erase(segments.begin() + k);
                k--;
                continue;              
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

float xNorm(float x)
{
    return (x - 1024) / 1024.0;
}

float yNorm(float y)
{
    return (y - 512) / 512.0;
}

void Image::InverseProject()
{
    /*float in[12] = {-4.72019703e+00, -7.05760046e+01, -3.79073247e+03,  3.78160553e+03,
                    1.28422170e-02, -1.20643492e+00, -6.81460386e+01,  6.79662635e+01,
                    6.58140711e-04, -2.22558522e-02, -1.00304332e+00,  1.00000000e+00};

    //#pragma omp parallel for
    for(size_t i = 0; i < segments.size(); i++)
    {
        float z, near = 0.1, far = 80.0, u, v, x, w;
        if(segments[i].label == "vegetation")
        {
            u = segments[i].box.x1;
            v = yNorm(segments[i].box.y2);
            z = (1 / segments[i].box.averageDepth - 1 / near) / ( 1 / far -  1 / near);
            if(u < 1024)
            {
                u = xNorm(u);
                x = in[0] * u + in[1] * v + in[2] * z + in[3] * 1.0;
                w = in[8] * u + in[9] * v + in[10] * z + in[11] * 1.0;

                segments[i].box.x1 = x / w; 
                segments[i].box.x2 = segments[i].box.x1 + 100;
            } 
            else
            {
                u = xNorm(segments[i].box.x2);
                x = in[0] * u + in[1] * v + in[2] * z + in[3] * 1.0;
                w = in[8] * u + in[9] * v + in[10] * z + in[11] * 1.0;

                segments[i].box.x2 = x / w;  
                segments[i].box.x1 = segments[i].box.x2 - 100;
            }
        }
        else
        {
            u = xNorm(segments[i].box.x1);
            v = yNorm(segments[i].box.y2);
            z = (1 / segments[i].box.averageDepth - 1 / near)/( 1 / far -  1 / near);

            x = in[0] * u + in[1] * v + in[2] * z + in[3] * 1.0;
            w = in[8] * u + in[9] * v + in[10] * z + in[11] * 1.0;

            segments[i].box.x1 = x / w;  

            u = xNorm(segments[i].box.x2);
            x = in[0] * u + in[1] * v + in[2] * z + in[3] * 1.0;
            w = in[8] * u + in[9] * v + in[10] * z + in[11] * 1.0;

            segments[i].box.x2 = x / w; 
        } 

        cout << segments[i].box.x1 << endl;
    }*/
    /* aachen
    float in[9] = {-1.18061356e+00, -2.90375747e+00,  2.58087415e+03,
                   -2.12429347e-05, -7.17727742e-03, -4.57078599e-01,
                    2.30509533e-05, -2.25309704e-03,  1.00000000e+00};*/
    // bochum
    float in[9] = {-1.47516046e+00, -2.55711911e+00,  2.56158105e+03,
        5.60180092e-04, -5.00376662e-03, -3.42637790e+00,
       -3.74517625e-05, -2.42506625e-03,  1.00000000e+00};

    //#pragma omp parallel for
    for(size_t i = 0; i < segments.size(); i++)
    {
        if(segments[i].label == "vegetation")
        {
            float u = segments[i].box.x1;
            float v = segments[i].box.y2;
            if(u < 500)
            {
                float x = in[0] * u + in[1] * v + in[2] * 1.0;
                float w = in[6] * u + in[7] * v + in[8] * 1.0;

                segments[i].box.x1 = x / w; 
                segments[i].box.x2 = segments[i].box.x1 + 10;
            } 
            else
            {
                u = segments[i].box.x2;
                float x = in[0] * u + in[1] * v + in[2] *1.0;
                float w = in[6] * u + in[7] * v + in[8] *1.0;

                segments[i].box.x2 = x / w;  
                segments[i].box.x1 = segments[i].box.x2 - 10;
            }
            continue;
        }

        if(segments[i].label == "sidewalk")
        {
            float u = segments[i].box.x1;
            float v = segments[i].box.y2;
            if(u < 500)
            {
                float x = in[0] * u + in[1] * v + in[2] * 1.0;
                float w = in[6] * u + in[7] * v + in[8] * 1.0;

                segments[i].box.x1 = x / w; 
                segments[i].box.x2 = segments[i].box.x1 + 100;
            } 
            else
            {
                u = segments[i].box.x2;
                float x = in[0] * u + in[1] * v + in[2] *1.0;
                float w = in[6] * u + in[7] * v + in[8] *1.0;

                segments[i].box.x2 = x / w;  
                segments[i].box.x1 = segments[i].box.x2 - 100;
            }
            continue;
        }

        float u = segments[i].box.x1;
        float v = segments[i].box.y2;

        float x = in[0] * u + in[1] * v + in[2] * 1.0;
        float w = in[6] * u + in[7] * v + in[8] * 1.0;

        segments[i].box.x1 = x / w;  

        u = segments[i].box.x2;
        x = in[0] * u + in[1] * v + in[2] *1.0;
        w = in[6] * u + in[7] * v + in[8] *1.0;

        segments[i].box.x2 = x / w;  
    }
}

void Image::ComputeBoundingBox()
{
    //#pragma omp parallel for
    for(size_t i = 0; i < segments.size(); i++)
    {
        segments[i].ComputeBoundingBox(segments[i].label);
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
                if(newSegment.label == "unlabeled" || newSegment.label == "static" ||  newSegment.label == "dynamic" || newSegment.label == "out of roi" || newSegment.label == "sky")
                    continue;
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