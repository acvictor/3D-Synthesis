#pragma once

#include <vector>
#include <string>
#include <iostream>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>
#include <assimp/config.h>

#include "Mesh.h"
#include "Texture.h"
#include "AABB.h"

class Model
{
public:
	float depth, xPos, yPos;
	float rotX, rotY, rotZ;
	float scale;
	float zSize, xSize, ySize;

	std::string label;
	AABB bb;

	Model();
	Model(float d, float x, float y);

	void SetValues(float d, float x, float y);
	void LoadModel(const std::string& fileName);
	void RenderModel();
	void ClearModel();

	~Model();

private:

	void LoadNode(aiNode *node, const aiScene *scene);
	void LoadMesh(aiMesh *mesh, const aiScene *scene);
	void LoadMaterials(const aiScene *scene);

	std::vector<Mesh*> meshList;
	std::vector<Texture*> textureList;
	std::vector<unsigned int> meshToTex;
};

