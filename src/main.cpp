#define STB_IMAGE_IMPLEMENTATION

#include "../include/Image.h"
#include "../include/Mesh.h"
#include "../include/Shader.h"
#include "../include/Texture.h"
#include "../include/PointLight.h"
#include "../include/SpotLight.h"
#include "../include/Material.h"
#include "../include/Model.h"
#include "../include/SceneGenerator.h"

using namespace std;

#define cout(a) cout<<a<<endl

int main(int argc, char** argv) 
{
    const char *s1 = argv[1];
    string fName1(s1);
    Image image;
    image.ReadJson(fName1);
    image.ComputeBoundingBox();
    image.DrawSegments(fName1); 
    const char *s2 = argv[2];
    string fName2(s2);
    image.GetDepth(fName2);   
    image.PrintSegments();

    SceneGenerator generator = SceneGenerator();
    generator.CreateRoad();
    generator.CreateShaders();
    generator.Render();

	return 0;
}

