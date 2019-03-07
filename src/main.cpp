#define STB_IMAGE_IMPLEMENTATION

#include "../include/SceneGenerator.h"

using namespace std;

int main(int argc, char** argv) 
{
    const char *s1 = argv[1];
    string fName1(s1);
    Image image = Image();
    image.ReadJson(fName1);
    image.ComputeBoundingBox();
    image.DrawSegments(fName1); 
    const char *s2 = argv[2];
    string fName2(s2);
    image.GetDepth(fName2);   
    //image.PrintDepth();

    SceneGenerator generator = SceneGenerator();
    generator.VerifyLocation(&image);
    image.InverseProject();
    generator.AddModels(image);
	while (!generator.mainWindow.getShouldClose())
	{
		generator.Render();
	}

	return 0;
}

