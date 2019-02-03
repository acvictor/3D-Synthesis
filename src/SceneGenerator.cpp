#include "../include/SceneGenerator.h"

const float toRadians = 3.14159265f / 180.0f;

SceneGenerator::SceneGenerator()
{
	deltaTime = 0.0f;
	lastTime = 0.0f;
	
	uniformProjection = 0; 
	uniformModel = 0; 
	uniformView = 0; 
	uniformEyePosition = 0;
	uniformSpecularIntensity = 0;
	uniformShininess = 0; 
	uniformFogColour = 0;

	glossy = Material(4.0f, 256);
	dull = Material(0.3f, 4);

	projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 100.0f);

	Init();
}

void SceneGenerator::Init()
{
	mainWindow = Window(2048, 1024);
	mainWindow.Initialise();

	camera = Camera(glm::vec3(0.0f, 2.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -60.0f, 0.0f, 5.0f, 0.5f);

	mainLight = DirectionalLight(2048, 2048,
								 1.0f, 1.0f, 1.0f, 
								 0.1f, 0.3f,
								 0.0f, -15.0f, -10.0f);
}

void SceneGenerator::AddModels(Image& image)
{
	for(int i = 0; i < image.segments.size(); i++)
	{

	}
}

void SceneGenerator::CreateRoad()
{
	unsigned int roadIndices[] = {
		0, 2, 1,
		1, 2, 3
	};

	GLfloat roadVertices[] = {
		-100.0f, 0.0f, -100.0f, 0.0f, 0.0f,	0.0f, -1.0f, 0.0f,
		 100.0f, 0.0f, -100.0f, 1.0f, 0.0f,	0.0f, -1.0f, 0.0f,
		 100.0f, 0.0f,  100.0f, 0.0f, 1.0f,	0.0f, -1.0f, 0.0f,
		-100.0f, 0.0f,  100.0f, 1.0f, 1.0f,	0.0f, -1.0f, 0.0f
	};

	Mesh *road = new Mesh();
	road->CreateMesh(roadVertices, roadIndices, 32, 6);
	meshList.push_back(road);
}

void SceneGenerator::CreateShaders()
{
	Shader *shader = new Shader();
	shader->CreateFromFiles("shaders/shader.vert", "shaders/shader.frag");
	shaderList.push_back(*shader);
	directionalShadowShader.CreateFromFiles("shaders/directionalShadowMap.vert", "shaders/directionalShadowMap.frag");
}

void SceneGenerator::TransformAndRenderMesh(Mesh* m, Material* mat, GLfloat transX, GLfloat transY, GLfloat transZ, GLfloat scale, GLfloat rotX, GLfloat rotY, GLfloat rotZ)
{
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(transX, transY, transZ));
	model = glm::rotate(model, rotX * toRadians, glm::vec3(1, 0, 0));
	model = glm::rotate(model, rotY * toRadians, glm::vec3(0, 1, 0));
	model = glm::rotate(model, rotZ * toRadians, glm::vec3(0, 0, 1));
	model = glm::scale(model, glm::vec3(scale, scale, scale));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	mat->UseMaterial(uniformSpecularIntensity, uniformShininess);
	m->RenderMesh();
}

void SceneGenerator::RenderScene()
{
	TransformAndRenderMesh(meshList[0], &dull, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f);
}

void SceneGenerator::DirectionalShadowMapPass(DirectionalLight* light)
{
	directionalShadowShader.UseShader();

	glViewport(0, 0, light->GetShadowMap()->GetShadowWidth(), light->GetShadowMap()->GetShadowHeight());

	light->GetShadowMap()->Write();
	glClear(GL_DEPTH_BUFFER_BIT);

	uniformModel = directionalShadowShader.GetModelLocation();
	glm::mat4 temp = light->CalculateLightTransform();
	directionalShadowShader.SetDirectionalLightTransform(&temp);

	RenderScene();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SceneGenerator::RenderPass(glm::mat4 viewMatrix)
{
	shaderList[0].UseShader();

	uniformModel = shaderList[0].GetModelLocation();
	uniformProjection = shaderList[0].GetProjectionLocation();
	uniformView = shaderList[0].GetViewLocation();
	uniformModel = shaderList[0].GetModelLocation();
	uniformEyePosition = shaderList[0].GetEyePositionLocation();
	uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
	uniformShininess = shaderList[0].GetShininessLocation();
	uniformFogColour = shaderList[0].GetFogColourLocation();

	glViewport(0, 0, mainWindow.getWidth(), mainWindow.getHeight());

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
	glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glUniform3f(uniformEyePosition, camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);
	glUniform4f(uniformFogColour, 0.4f, 0.4f, 0.4f, 1.0f);

	shaderList[0].SetDirectionalLight(&mainLight);
	shaderList[0].SetPointLights(pointLights, 0);
	shaderList[0].SetSpotLights(spotLights, 0);
	glm::mat4 temp = mainLight.CalculateLightTransform();
	shaderList[0].SetDirectionalLightTransform(&temp);

	mainLight.GetShadowMap()->Read(GL_TEXTURE1);
	shaderList[0].SetTexture(0);
	shaderList[0].SetDirectionalShadowMap(1);

	glm::vec3 lowerLight = camera.getCameraPosition();
	lowerLight.y -= 0.3f;
	//spotLights[0].SetFlash(lowerLight, camera.getCameraDirection());

	RenderScene();
}

void SceneGenerator::Render()
{
	while (!mainWindow.getShouldClose())
	{
		GLfloat now = glfwGetTime(); ;
		deltaTime = now - lastTime; 
		lastTime = now;

		glfwPollEvents();

		camera.keyControl(mainWindow.getsKeys(), deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

		DirectionalShadowMapPass(&mainLight);
		RenderPass(camera.calculateViewMatrix());

		mainWindow.swapBuffers();
	}
}