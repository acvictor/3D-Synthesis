#pragma once

#include "../include/Window.h"
#include "../include/Camera.h"
#include "../include/DirectionalLight.h"

class SceneGenerator
{
public:
	Window mainWindow;
	Camera camera;
	DirectionalLight mainLight;

	SceneGenerator();

	void Init();
};