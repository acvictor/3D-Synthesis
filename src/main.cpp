#include "../include/Image.h"

using namespace std;

#define cout(a) cout<<a<<endl

int main(int argc, char** argv) 
{
    const char *s1 = argv[1];
    string fName1(s1);
    Image image;
    image.ReadJson(fName1);
    //image.PrintSegments();
    image.ComputeBoundingBox();
    image.DrawSegments(fName1); 
    const char *s2 = argv[2];
    string fName2(s2);
    cout(fName2);
    image.GetDepth(fName2);   

	return 0;
}

