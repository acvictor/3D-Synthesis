#include "../include/SceneGenerator.h"

static const char* vShader = "shaders/shader.vert";
static const char* fShader = "shaders/shader.frag";

const float toRadians = 3.14159265f / 180.0f;
const float reSize = 70;
const float xFactor = 5.0f;
const float zFactor = 2.5f;
const float uniScale = 11.0f;
float sidewalk[2];
int sidewalkInd[2];

using namespace std;

bool SceneGenerator::lessThan(const Segment a, const Segment b)
{
	return (a.box.y2 < b.box.y2);
}

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

bool SceneGenerator::OutOfInterest(int i)
{
	if(modelList[i].label == "sidewalk" || modelList[i].label == "vegetation" || modelList[i].label == "ground" || modelList[i].label == "road" || modelList[i].label == "sidewalk" || modelList[i].label == "license plate")
	{
		return 1;
	}
	else
	{
		return 0;
	}
	
}

void SceneGenerator::CollisionDetection()
{
	for(int i = 0; i < modelList.size(); i++)
	{
		modelList[i].xSize *=  uniScale * modelList[i].scaleX;
		modelList[i].bb.minx = modelList[i].xPos - modelList[i].xSize / 2.0;
		modelList[i].bb.maxx = modelList[i].xPos + modelList[i].xSize / 2.0;

		modelList[i].zSize *=  uniScale * modelList[i].scaleZ;
		modelList[i].bb.minz = modelList[i].depth + modelList[i].zSize / 2.0;
		modelList[i].bb.maxz = modelList[i].depth - modelList[i].zSize / 2.0;
	}

	for(int i = 0; i < modelList.size(); i++)
	{
		if(modelList[i].label == "car")
		{
			if(modelList[i].xPos < 0)
			{
				if(modelList[i].xPos - modelList[i].xSize < modelList[sidewalkInd[0]].bb.minx)
				{
					sidewalk[0] -= fabs(modelList[i].xPos - modelList[sidewalkInd[0]].bb.minx + 10);
					modelList[sidewalkInd[0]].xPos = sidewalk[0];
				}
			}
			else
			{
				if(modelList[i].xPos + modelList[i].xSize > modelList[sidewalkInd[1]].bb.minx)
				{
					sidewalk[1] += fabs(modelList[i].xPos - modelList[sidewalkInd[1]].bb.minx + 10);
					modelList[sidewalkInd[1]].xPos = sidewalk[1];
				}
			}	
		}
	}

	for(int i = 0; i < modelList.size(); i++)
	{
		if(modelList[i].label == "vegetation")
		{
			if(modelList[i].xPos < 0)
			{
				if(modelList[i].xPos > sidewalk[0])
				{
					modelList[i].xPos -= fabs(modelList[i].xPos - sidewalk[0] + 20);
				}
			}
			else
			{
				if(modelList[i].xPos < sidewalk[1])
				{
					modelList[i].xPos += fabs(modelList[i].xPos - sidewalk[1] + 20);
				}
			}
			
		}
	}

	int axis = 2;
	for(int i = 0; i < modelList.size(); i++)
	{
		if(OutOfInterest(i))
			continue;
		for(int j = 0; j < modelList.size(); j++)
		{
			if(j == i || OutOfInterest(j))
				continue;

			float move = modelList[i].bb.Intersect(modelList[j].bb, axis);
			modelList[i].depth += move;
		}		
	}

	for(int i = 0; i < modelList.size(); i++)
	{
		if(modelList[i].label == "building")
		{
			for(int j = 0; j < modelList.size(); j++)
			{
				if(modelList[j].label == "vegetation")
				{
					if(modelList[i].xPos < 0 && modelList[j].xPos < 0 && modelList[i].bb.maxx > modelList[j].xPos)
					{
						float move = fabs(modelList[j].xPos - modelList[i].xPos) + modelList[i].xSize / 1.5;
						modelList[i].xPos -= move;
						cout << modelList[i].xPos << " " <<  modelList[j].xPos << endl;
						break;
					}

					else if(modelList[i].xPos > 0 && modelList[j].xPos > 0 && modelList[i].bb.minx < modelList[j].xPos)
					{
						float move = fabs(modelList[j].xPos - modelList[i].xPos) + modelList[i].xSize / 1.5;
						modelList[i].xPos += move;
						break;
					}
				}
			}
		}
	}
}

void SceneGenerator::VerifyLocation(Image* image)
{	
	std::sort(image->segments.begin(), image->segments.end(), lessThan);

	for(size_t i = 1; i < image->segments.size(); i++)
	{
		// Y increases downwards
		if(fabs(image->segments[i].box.y2 - image->segments[i - 1].box.y2) < 10)
		{
			if(image->segments[i].box.averageDepth > image->segments[i - 1].box.averageDepth)
			{
				image->segments[i].box.averageDepth = image->segments[i - 1].box.averageDepth = (image->segments[i].box.averageDepth + image->segments[i - 1].box.averageDepth) / 2.0;
			}
			else if(fabs(image->segments[i].box.y2 - image->segments[i - 1].box.y2) < 5 && fabs(image->segments[i].box.averageDepth - image->segments[i - 1].box.averageDepth) > 1.0)
			{
				image->segments[i].box.averageDepth = image->segments[i - 1].box.averageDepth;
			}
		}
	}	
}

void SceneGenerator::AddModels(Image image)
{
	int c = 0;
	for(size_t i = 0; i < image.segments.size(); i++)
	{
		if(image.segments[i].label == "car")
		{
			float temp = image.segments[i].box.averageDepth * zFactor;
			int x = rand() % 2;
			Model* newModel = new Model(temp, 
								      ((image.segments[i].box.x1 + image.segments[i].box.x2) / 2.0f - 1024) / reSize, 
								        0.0f);
			newModel->label = image.segments[i].label;
			if(x == 0)
			{
				newModel->LoadModel("assets/car/car.obj");
				newModel->rotX = -90.0f;
				newModel->rotZ = -90.0f;
				newModel->SetScale(1.2f);
				newModel->yPos = 5.0f;
				swap(newModel->ySize, newModel->xSize);
				swap(newModel->ySize, newModel->zSize);				
			}
			else if(x == 1)
			{
				newModel->LoadModel("assets/SUV/SUV.obj");
				newModel->rotX = -90.0f;
				newModel->rotZ = -180.0f;
				newModel->SetScale(1.5f);
				newModel->yPos = 6.5f;
				swap(newModel->zSize, newModel->ySize);
			}
			modelList.push_back(*newModel);
		}

		else if(image.segments[i].label == "person")
		{
			float temp = image.segments[i].box.averageDepth * zFactor;
			Model* newModel = new Model(temp, 
								      ((image.segments[i].box.x1 + image.segments[i].box.x2) / 2.0f - 1024) / reSize, 
								        0.0f);
			newModel->label = image.segments[i].label;
			newModel->LoadModel("assets/person/person.obj");
			newModel->rotY = -90.0f;
			newModel->SetScale(0.6f);
			newModel->yPos = 7.0f;
			swap(newModel->xSize, newModel->zSize);
			modelList.push_back(*newModel);
		}

		/*else if(image.segments[i].label == "pole")
		{
			float temp = image.segments[i].box.averageDepth * zFactor;
			Model* newModel = new Model(temp, 
								      ((image.segments[i].box.x1 + image.segments[i].box.x2) / 2.0f - 1024) / reSize, 
								        0.0f);
			newModel->label = image.segments[i].label;
			newModel->LoadModel("assets/pole/pole.obj");
			newModel->yPos = 25.0f;
			newModel->SetScale(1, 3, 1);
			modelList.push_back(*newModel);
		}*/


		else if(image.segments[i].label == "vegetation")
		{
			Model* model = new Model();
			model->LoadModel("assets/tree2/Tree.obj");
			model->label = image.segments[i].label;
			float size = 5;
			int no = (std::min(400.0f, image.segments[i].box.maxDepth) - image.segments[i].box.minDepth) / (size * 1.5);
			
			for(int j = 0; j < no; j++)
			{
				float temp = image.segments[i].box.minDepth * 1.5 + j * size * uniScale;
			
				if(temp > 400)
					break;

				model->SetValues(temp, 
								((image.segments[i].box.x1 + image.segments[i].box.x2) / 2.0f - 1024) / reSize, 
								0.0f); 
				model->SetScale(static_cast <float> (rand()) / static_cast <float> (RAND_MAX) + 4);
				model->yPos = uniScale * model->scaleX;				
				modelList.push_back(*model);
			}
		}

		else if(image.segments[i].label == "sidewalk")
		{
			Model* model = new Model();
			float temp = 0 * zFactor;
			model->LoadModel("assets/side/sidewalk.obj");
			model->label = image.segments[i].label;
			model->SetValues(temp, 
							((image.segments[i].box.x1 + image.segments[i].box.x2) / 2.0f - 1024) / reSize * 1.8, 
							0.0f); 
			model->SetScale(8, 10, 400);
			model->yPos = 0.0f;				
			modelList.push_back(*model);
			if(model->xPos < 0)
			{
				sidewalk[0] = model->xPos;
				sidewalkInd[0] = modelList.size() - 1;
			}
			else
			{
				sidewalk[1] = model->xPos;
				sidewalkInd[1] = modelList.size() - 1;
			}	
		}
		
		else if(image.segments[i].label == "building")
		{
			Model* model = new Model();
			model->LoadModel("assets/building/building.obj");
			model->label = image.segments[i].label;
			float size = 2;
			int no = (std::min(400.0f, image.segments[i].box.maxDepth) - image.segments[i].box.minDepth) / (size * 1.5);
			
			for(int j = 0; j < no; j++)
			{
				float temp = image.segments[i].box.minDepth * 1.5 + j * size * uniScale;
			
				if(temp > 400)
					break;

				model->SetValues(temp, 
								((image.segments[i].box.x1 + image.segments[i].box.x2) / 2.0f - 1024) / reSize, 
								0.0f); 
				model->SetScale(static_cast <float> (rand()) / static_cast <float> (RAND_MAX) + 4);
				model->yPos = uniScale * model->scaleX;				
				modelList.push_back(*model);
			}			
		}
	}
	CollisionDetection();
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

void SceneGenerator::TransformAndRenderModel(Model* m, Material* mat, GLfloat transX, GLfloat transY, GLfloat transZ, GLfloat scaleX, GLfloat scaleY, GLfloat scaleZ, GLfloat rotX, GLfloat rotY, GLfloat rotZ)
{
	glm::mat4 model = glm::mat4();
	model = glm::translate(model, glm::vec3(transX, transY, transZ));
	model = glm::rotate(model, rotX * toRadians, glm::vec3(1, 0, 0));
	model = glm::rotate(model, rotY * toRadians, glm::vec3(0, 1, 0));
	model = glm::rotate(model, rotZ * toRadians, glm::vec3(0, 0, 1));
	model = glm::scale(model, glm::vec3(scaleX, scaleY, scaleZ));
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
	TransformAndRenderMesh(meshList[0], &dullMaterial, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f);

	for(size_t i = 0; i < modelList.size(); i++)
	{
		if(modelList[i].label == "car")
		{
			TransformAndRenderModel(&modelList[i], &shinyMaterial, modelList[i].xPos, modelList[i].yPos, modelList[i].depth, modelList[i].scaleX * uniScale, modelList[i].scaleY * uniScale, modelList[i].scaleZ * uniScale, modelList[i].rotX, modelList[i].rotY, modelList[i].rotZ);
		}
		else
			TransformAndRenderModel(&modelList[i], &dullMaterial, modelList[i].xPos, modelList[i].yPos, modelList[i].depth, modelList[i].scaleX * uniScale, modelList[i].scaleY * uniScale, modelList[i].scaleZ * uniScale, modelList[i].rotX, modelList[i].rotY, modelList[i].rotZ);
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
	mainWindow = Window(1300, 650);
	mainWindow.Initialise();

	CreateObjects();
	CreateShaders();

	camera = Camera(glm::vec3(0.0f, 4.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 20.0f, 0.5f);

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

    projection = glm::perspective(toRadians * 70.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 400.0f);
	//projection = glm::ortho(-150.0f, 150.0f, -1.0f, 150.0f, -400.f, 400.0f);
}