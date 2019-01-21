#include "../include/Image.h"
#include "../include/Label.h"

#define cout(a) cout<<a<<endl

bool drawBox = 1;

Image::Image()
{
	imgHeight = 0;
	imgWidth = 0;
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
        cout << segments[i].label << " " << segments[i].pointCount << endl;
        for(size_t j = 0; j < segments[i].polygon.size(); j++)
        {
            cout << segments[i].polygon[j].x << " " << segments[i].polygon[j].y << endl;
        }
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