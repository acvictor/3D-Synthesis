#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../include/Window.h"
#include "../include/Camera.h"
#include "../include/Material.h"
#include "../include/Shader.h"
#include "../include/Texture.h"
#include "../include/DirectionalLight.h"
#include "../include/Model.h"
#include "../include/Image.h"
#include "../include/PointLight.h"
#include "../include/SpotLight.h"
#include "../include/CommonValues.h"

class SceneGenerator
{
public:
	Window mainWindow;
	Camera camera;

	DirectionalLight mainLight;
	PointLight pointLights[MAX_POINT_LIGHTS];
	SpotLight spotLights[MAX_SPOT_LIGHTS];

	std::vector<Model> modelList;
	std::vector<Mesh*> meshList;
	std::vector<Texture> textureList;

	std::vector<Shader> shaderList;
	Shader directionalShadowShader;

	Material glossy, dull;

	GLfloat deltaTime, lastTime;
	
	GLuint uniformProjection, uniformModel, uniformView, uniformEyePosition, uniformSpecularIntensity, uniformShininess, uniformFogColour;

	glm::mat4 projection;

	SceneGenerator();

	void Init();
	void AddModels(Image& image);
	void CreateRoad();
	void CreateShaders();

	void TransformAndRenderMesh(Mesh* m, Material* mat, GLfloat transX, GLfloat transY, GLfloat transZ, GLfloat scale, GLfloat rotX, GLfloat rotY, GLfloat rotZ);
	void RenderScene();
	void DirectionalShadowMapPass(DirectionalLight* light);
	void RenderPass(glm::mat4 viewMatrix);
	void Render();
};