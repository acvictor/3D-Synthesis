#include "../include/Image.h"
#include "../include/Label.h"

#define cout(a) cout<<a<<endl

bool drawBox = 1;

Image::Image()
{
	imgHeight = 0;
	imgWidth = 0;
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

    if (file.is_open())
    {
        float v;
        for (size_t i = 0; i < imgWidth; i++)
        {
            for(size_t j = 0; j < imgHeight; j++)
            {
                file >> depth[i][j];
            }
        }

        #pragma omp parallel for
        for(size_t k = 0; k < segments.size(); k++)
        {
            std::map<int, int> depthRep;
            float averageDepth = 0.0;

            int xStart = (int)floor(segments[k].box.x1);
            int yStart = (int)floor(segments[k].box.y1);
            int xStop = (int)ceil(segments[k].box.x2);
            int yStop = (int)ceil(segments[k].box.y2);

            #pragma omp parallel for collapse(2)
            for(size_t i = xStart; i < xStop; i++)
            {
                for(size_t j = yStart; j < yStop; j++)
                {
                    depthRep[(int)ceil(depth[i][j])] += 1;
                    averageDepth += depth[i][j];
                    segments[k].box.minDepth = min(segments[k].box.minDepth, depth[i][j]);
                    segments[k].box.maxDepth = max(segments[k].box.maxDepth, depth[i][j]);
                }
            }

            averageDepth /= (float)((xStop - xStart) * (yStop - yStart));
            segments[k].box.averageDepth = averageDepth;

            vector<pair<int, int> > mapcopy(depthRep.begin(), depthRep.end());
            sort(mapcopy.begin(), mapcopy.end(), lessSecond<int, int>());  
            segments[k].box.mostRepDepth = mapcopy[0].first;
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
    cout(fName);

    Mat image = Mat::zeros(imgHeight, imgWidth, CV_8UC3 );

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
        fillPoly(image, ppt, npt, 1, Scalar(col.r, col.g, col.b), 8 );

        if(drawBox)
            rectangle(image, Point(segments[i].box.x1, segments[i].box.y1), Point(segments[i].box.x2, segments[i].box.y2), Scalar(255, 255, 255));
    }

    imshow("Image", image);
    waitKey(0);
}

void Image::PrintSegments()
{
    cout(imgHeight);
    cout(imgWidth);
    for(size_t i = 0; i < segments.size(); i++)
    {
        cout << segments[i].label << "\nAverge depth = " << segments[i].box.averageDepth << 
                                     "\nMost repeated = " << segments[i].box.mostRepDepth <<
                                     "\nMin depth = " << segments[i].box.minDepth << 
                                     "\nMax depth = " << segments[i].box.maxDepth << endl;

        /*for(size_t j = 0; j < segments[i].polygon.size(); j++)
        {
            cout << segments[i].polygon[j].x << " " << segments[i].polygon[j].y << endl;
        }*/
        cout << endl;
    }
    cout << endl;
}

void Image::ComputeBoundingBox()
{
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
                imgHeight =stoi(line.substr(pos, line.length() - pos - 1)); 
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