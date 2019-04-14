#pragma once

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../include/CommonValues.h"
#include "../include/Window.h"
#include "../include/Image.h"
#include "../include/Mesh.h"
#include "../include/Shader.h"
#include "../include/Camera.h"
#include "../include/Texture.h"
#include "../include/DirectionalLight.h"
#include "../include/PointLight.h"
#include "../include/SpotLight.h"
#include "../include/Material.h"
#include "../include/Model.h"

class SceneGenerator
{
public:
    Window mainWindow;
    std::vector<Mesh*> meshList;
    std::vector<Shader> shaderList;
    Shader directionalShadowShader;

    std::vector<Model> modelList;
    Material shinyMaterial, dullMaterial;

    Camera camera;

    Texture brickTexture;
    Texture marbleTexture;
    Texture plainTexture;
    
    DirectionalLight mainLight;
    PointLight pointLights[MAX_POINT_LIGHTS];
    SpotLight spotLights[MAX_SPOT_LIGHTS];

    GLfloat deltaTime;
    GLfloat lastTime;

    glm::mat4 projection;

    unsigned int pointLightCount, spotLightCount;

    GLuint uniformProjection, uniformModel, uniformView, uniformEyePosition, uniformSpecularIntensity, uniformShininess, uniformFogColour;

    SceneGenerator();
    void Init();

    void calcAverageNormals(unsigned int * indices, unsigned int indiceCount, GLfloat * vertices, unsigned int verticeCount, 
						unsigned int vLength, unsigned int normalOffset);
    void CreateObjects();
    void CreateShaders();

    bool OutOfInterest(int);
    void CollisionDetection();

    void AddModels(Image image);
    static bool lessThan(const Segment a, const Segment b);
    void VerifyLocation(Image* image);

    void TransformAndRenderModel(Model* m, Material* mat, GLfloat transX, GLfloat transY, GLfloat transZ, GLfloat scaleX, GLfloat scaleY, GLfloat scaleZ, GLfloat rotX, GLfloat rotY, GLfloat rotZ);
    void TransformAndRenderMesh(Mesh* m, Material* mat, GLfloat transX, GLfloat transY, GLfloat transZ, GLfloat scale, GLfloat rotX, GLfloat rotY, GLfloat rotZ);
    void RenderSceneGenerator();
    void DirectionalShadowMapPass();
    void RenderPass(glm::mat4 viewMatrix);
    void Render();
};