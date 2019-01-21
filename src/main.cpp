#include "../include/Image.h"

using namespace std;

#define cout(a) cout<<a<<endl

int main(int argc, char** argv) 
{
    const char *s = argv[1];
    string fName(s);
    Image image;
    image.ReadJson(fName);
    image.PrintSegments();
    image.ComputeBoundingBox();
    image.DrawSegments(fName);    

	return 0;
}

