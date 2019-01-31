#include "../include/SceneGenerator.h"

SceneGenerator::SceneGenerator()
{
	Init();
}

void SceneGenerator::Init()
{
	mainWindow = Window(1366, 768); // 1280, 1024 or 1024, 768
	mainWindow.Initialise();

	camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -60.0f, 0.0f, 5.0f, 0.5f);

	mainLight = DirectionalLight(2048, 2048,
								 1.0f, 1.0f, 1.0f, 
								 0.1f, 0.3f,
								 0.0f, -15.0f, -10.0f);
}