#include "../include/SceneGenerator.h"

static const char* vShader = "shaders/shader.vert";
static const char* fShader = "shaders/shader.frag";

const float toRadians = 3.14159265f / 180.0f;
const float reSize = 10;

using namespace std;

SceneGenerator::SceneGenerator()
{
    uniformProjection = 0; 
    uniformModel = 0; 
    uniformView = 0; 
    uniformEyePosition = 0; 
    uniformSpecularIntensity = 0; 
    uniformShininess = 0; 
    uniformFogColour = 0;

    pointLightCount = 0; spotLightCount = 0;

    deltaTime = 0.0f;
    lastTime = 0.0f;

    Init();
}

void SceneGenerator::AddModels(Image image)
{
	float aspect = (float)mainWindow.getBufferWidth() / (float)mainWindow.getBufferHeight();
	for(size_t i = 0; i < image.segments.size(); i++)
	{
		if(image.segments[i].label == "car")
		{
			int x = rand() % 2;
			Model* newModel = new Model(image.segments[i].box.averageDepth * 4, 
								      ((image.segments[i].box.x1 + image.segments[i].box.x2) / 2.0f - 1024) / reSize, 
								        0.0f);
			if(x == 0)
			{
				newModel->LoadModel("assets/car/car.obj");
				newModel->rotX = -90.0f;
				newModel->rotZ = -90.0f;
			}
			else if(x == 1)
			{
				newModel->LoadModel("assets/SUV/SUV.obj");
				newModel->rotX = -90.0f;
				newModel->rotZ = -180.0f;
			}
			modelList.push_back(*newModel);
		}

		if(image.segments[i].label == "person")
		{
			Model* newModel = new Model(image.segments[i].box.averageDepth * 2, 
								      ((image.segments[i].box.x1 + image.segments[i].box.x2) / 2.0f - 1024) / reSize, 
								        0.0f);
			newModel->LoadModel("assets/person/person.obj");
			newModel->rotY = -90.0f;
			newModel->scale = 0.4f;
			cout << newModel->xPos << endl;
			modelList.push_back(*newModel);
		}
	}
	
}

void SceneGenerator::calcAverageNormals(unsigned int * indices, unsigned int indiceCount, GLfloat * vertices, unsigned int verticeCount, 
						unsigned int vLength, unsigned int normalOffset)
{
	for (size_t i = 0; i < indiceCount; i += 3)
	{
		unsigned int in0 = indices[i] * vLength;
		unsigned int in1 = indices[i + 1] * vLength;
		unsigned int in2 = indices[i + 2] * vLength;
		glm::vec3 v1(vertices[in1] - vertices[in0], vertices[in1 + 1] - vertices[in0 + 1], vertices[in1 + 2] - vertices[in0 + 2]);
		glm::vec3 v2(vertices[in2] - vertices[in0], vertices[in2 + 1] - vertices[in0 + 1], vertices[in2 + 2] - vertices[in0 + 2]);
		glm::vec3 normal = glm::cross(v1, v2);
		normal = glm::normalize(normal);
		
		in0 += normalOffset; in1 += normalOffset; in2 += normalOffset;
		vertices[in0] += normal.x; vertices[in0 + 1] += normal.y; vertices[in0 + 2] += normal.z;
		vertices[in1] += normal.x; vertices[in1 + 1] += normal.y; vertices[in1 + 2] += normal.z;
		vertices[in2] += normal.x; vertices[in2 + 1] += normal.y; vertices[in2 + 2] += normal.z;
	}

	for (size_t i = 0; i < verticeCount / vLength; i++)
	{
		unsigned int nOffset = i * vLength + normalOffset;
		glm::vec3 vec(vertices[nOffset], vertices[nOffset + 1], vertices[nOffset + 2]);
		vec = glm::normalize(vec);
		vertices[nOffset] = vec.x; vertices[nOffset + 1] = vec.y; vertices[nOffset + 2] = vec.z;
	}
}

void SceneGenerator::CreateObjects() 
{
	unsigned int floorIndices[] = {
		0, 2, 1,
		1, 2, 3
	};

	GLfloat floorVertices[] = {
		-1000.0f, 0.0f, -1000.0f, 0.0f, 0.0f,	0.0f, -1.0f, 0.0f,
		 1000.0f, 0.0f, -1000.0f, 1.0f, 0.0f,	0.0f, -1.0f, 0.0f,
		-1000.0f, 0.0f,  1000.0f, 0.0f, 1.0f,	0.0f, -1.0f, 0.0f,
		 1000.0f, 0.0f,  1000.0f, 1.0f, 1.0f,	0.0f, -1.0f, 0.0f
	};

	unsigned int backWallIndices[] = {
		0, 2, 1,
		1, 2, 3
	};

	GLfloat backWallVertices[] = {
		-100.0f,   0.0f, -100.0f, 0.0f, 0.0f,	0.0f, 0.0f, -1.0f,
		 100.0f,   0.0f, -100.0f, 1.0f, 0.0f,	0.0f, 0.0f, -1.0f,
		-100.0f, 100.0f, -100.0f, 0.0f, 1.0f,	0.0f, 0.0f, -1.0f,
		 100.0f, 100.0f, -100.0f, 1.0f, 1.0f,	0.0f, 0.0f, -1.0f
	};

	Mesh *floor = new Mesh();
	floor->CreateMesh(floorVertices, floorIndices, 32, 6);
	meshList.push_back(floor);

	Mesh* wall = new Mesh();
	wall->CreateMesh(backWallVertices, backWallIndices, 32, 6);
	meshList.push_back(wall);
}

void SceneGenerator::TransformAndRenderModel(Model* m, Material* mat, GLfloat transX, GLfloat transY, GLfloat transZ, GLfloat scale, GLfloat rotX, GLfloat rotY, GLfloat rotZ)
{
	glm::mat4 model = glm::mat4();
	model = glm::translate(model, glm::vec3(transX, transY, transZ));
	model = glm::rotate(model, rotX * toRadians, glm::vec3(1, 0, 0));
	model = glm::rotate(model, rotY * toRadians, glm::vec3(0, 1, 0));
	model = glm::rotate(model, rotZ * toRadians, glm::vec3(0, 0, 1));
	model = glm::scale(model, glm::vec3(scale, scale, scale));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	mat->UseMaterial(uniformSpecularIntensity, uniformShininess);
	m->RenderModel();
}

void SceneGenerator::TransformAndRenderMesh(Mesh* m, Material* mat, GLfloat transX, GLfloat transY, GLfloat transZ, GLfloat scale, GLfloat rotX, GLfloat rotY, GLfloat rotZ)
{
	glm::mat4 model = glm::mat4();
	model = glm::translate(model, glm::vec3(transX, transY, transZ));
	model = glm::rotate(model, rotX * toRadians, glm::vec3(1, 0, 0));
	model = glm::rotate(model, rotY * toRadians, glm::vec3(0, 1, 0));
	model = glm::rotate(model, rotZ * toRadians, glm::vec3(0, 0, 1));
	model = glm::scale(model, glm::vec3(scale, scale, scale));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	plainTexture.UseTexture();
	mat->UseMaterial(uniformSpecularIntensity, uniformShininess);
	m->RenderMesh();
}

void SceneGenerator::CreateShaders()
{
	Shader *shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);
	directionalShadowShader.CreateFromFiles("shaders/directionalShadowMap.vert", "shaders/directionalShadowMap.frag");
}

void SceneGenerator::RenderSceneGenerator()
{
	glm::mat4 model;	
	TransformAndRenderMesh(meshList[0], &dullMaterial, 0.0f, -4.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f);
	//TransformAndRenderMesh(meshList[1], &dullMaterial, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f);

	for(size_t i = 0; i < modelList.size(); i++)
	{
		TransformAndRenderModel(&modelList[i], &dullMaterial, modelList[i].xPos, modelList[i].yPos, modelList[i].depth, modelList[i].scale * 10, modelList[i].rotX, modelList[i].rotY, modelList[i].rotZ);
	}
	
}


void SceneGenerator::DirectionalShadowMapPass()
{
	directionalShadowShader.UseShader();

	glViewport(0, 0, mainLight.GetShadowMap()->GetShadowWidth(), mainLight.GetShadowMap()->GetShadowHeight());

	mainLight.GetShadowMap()->Write();
	glClear(GL_DEPTH_BUFFER_BIT);

	uniformModel = directionalShadowShader.GetModelLocation();
	glm::mat4 temp = mainLight.CalculateLightTransform();
	directionalShadowShader.SetDirectionalLightTransform(&temp);

	RenderSceneGenerator();

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

	glClearColor(0.9f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
	glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glUniform3f(uniformEyePosition, camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);
	glUniform4f(uniformFogColour, 0.4f, 0.4f, 0.4f, 1.0f);

	shaderList[0].SetDirectionalLight(&mainLight);
	shaderList[0].SetPointLights(pointLights, pointLightCount);
	shaderList[0].SetSpotLights(spotLights, spotLightCount);
	glm::mat4 temp = mainLight.CalculateLightTransform();
	shaderList[0].SetDirectionalLightTransform(&temp);

	mainLight.GetShadowMap()->Read(GL_TEXTURE1);
	shaderList[0].SetTexture(0);
	shaderList[0].SetDirectionalShadowMap(1);

	glm::vec3 lowerLight = camera.getCameraPosition();
	lowerLight.y -= 0.3f;

	RenderSceneGenerator();
}

void SceneGenerator::Render()
{
	GLfloat now = glfwGetTime(); 
	deltaTime = now - lastTime;
	lastTime = now;

	glfwPollEvents();

	camera.keyControl(mainWindow.getsKeys(), deltaTime);
	camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

	DirectionalShadowMapPass();
	RenderPass(camera.calculateViewMatrix());

	mainWindow.swapBuffers();
}

void SceneGenerator::Init()
{
	mainWindow = Window(1600, 800);
	mainWindow.Initialise();

	CreateObjects();
	CreateShaders();

	camera = Camera(glm::vec3(0.0f, 2.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 60.0f, 0.0f, 10.0f, 0.5f);

	brickTexture = Texture("textures/brick.png");
	brickTexture.LoadTextureA();
	marbleTexture  = Texture("textures/marble.jpg");
	marbleTexture.LoadTexture();
	plainTexture = Texture("textures/plain.png");
	plainTexture.LoadTextureA();

	shinyMaterial = Material(4.0f, 256);
	dullMaterial = Material(0.3f, 4);

	mainLight = DirectionalLight(2048, 2048,
								1.0f, 1.0f, 1.0f, 
								0.6f, 0.3f,
								0.0f, -15.0f, -10.0f);

    projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 400.0f);
}