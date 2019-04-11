#include "../include/Model.h"

#define AI_CONFIG_PP_PTV_NORMALIZE "PP_PTV_NORMALIZE"

Model::Model()
{
	depth = 0.0f;
	rotX = rotY = rotZ = 0.0f;
	xPos = 0.0f;
	yPos = 0.0f;
	scale = 1.0f;
	bb = AABB();
}

Model::Model(float d, float x, float y)
{
	depth = -d;
	xPos = x;
	yPos = y;
	rotX = rotY = rotZ = 0.0f;
	scale = 1.0f;
}

void Model::SetValues(float d, float x, float y)
{
	depth = -d;
	xPos = x;
	yPos = y;
	rotX = rotY = rotZ = 0.0f;
	scale = 1.0f;
}

void Model::RenderModel()
{
	for (size_t i = 0; i < meshList.size(); i++)
	{
		unsigned int materialIndex = meshToTex[i];

		if (materialIndex < textureList.size() && textureList[materialIndex])
		{
			textureList[materialIndex]->UseTexture();
		}

		meshList[i]->RenderMesh();
	}
}

void Model::LoadModel(const std::string & fileName)
{
	Assimp::Importer importer;
	aiPropertyStore* props = aiCreatePropertyStore();

	// Normalizing model mesh
	aiSetImportPropertyInteger(props, "PP_PTV_NORMALIZE", 1);

	const aiScene* scene = (aiScene*)aiImportFileExWithProperties(fileName.c_str(),
	aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices | aiProcess_PreTransformVertices,
    NULL,
    props);
	aiReleasePropertyStore(props);
	
	if (!scene)
	{
		printf("Model (%s) failed to load: %s", fileName, importer.GetErrorString());
		return;
	}

	LoadNode(scene->mRootNode, scene);

	LoadMaterials(scene);
}

void Model::LoadNode(aiNode * node, const aiScene * scene)
{
	for (size_t i = 0; i < node->mNumMeshes; i++)
	{
		LoadMesh(scene->mMeshes[node->mMeshes[i]], scene);
	}

	for (size_t i = 0; i < node->mNumChildren; i++)
	{
		LoadNode(node->mChildren[i], scene);
	}
}

void Model::LoadMesh(aiMesh * mesh, const aiScene * scene)
{
	std::vector<GLfloat> vertices;
	std::vector<unsigned int> indices;

	GLfloat maxZ = (float)(INT_MIN), minZ = (float)INT_MAX;
	GLfloat maxY = (float)(INT_MIN), minY = (float)INT_MAX;
	GLfloat maxX = (float)(INT_MIN), minX = (float)INT_MAX;

	for (size_t i = 0; i < mesh->mNumVertices; i++)
	{
		vertices.insert(vertices.end(), { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z });

		maxZ = std::max(maxZ, mesh->mVertices[i].z); minZ = std::min(minZ, mesh->mVertices[i].z);
		maxY = std::max(maxY, mesh->mVertices[i].y); minY = std::min(minY, mesh->mVertices[i].y);
		maxX = std::max(maxX, mesh->mVertices[i].x); minX = std::min(minX, mesh->mVertices[i].x);

		if (mesh->mTextureCoords[0]) 
		{
			vertices.insert(vertices.end(), { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y });
		}
		else 
		{
			vertices.insert(vertices.end(), { 0.0f, 0.0f });
		}
		vertices.insert(vertices.end(), { -mesh->mNormals[i].x, -mesh->mNormals[i].y, -mesh->mNormals[i].z });
	}

	this->zSize = maxZ - minZ;
	this->ySize = maxY - minY;
	this->xSize = maxX - minX;
	
	this->bb = AABB(minX, maxX, minY, maxY, minZ, maxZ);

	for (size_t i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (size_t j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	Mesh* newMesh = new Mesh();
	newMesh->CreateMesh(&vertices[0], &indices[0], vertices.size(), indices.size());
	meshList.push_back(newMesh);
	meshToTex.push_back(mesh->mMaterialIndex);
}

void Model::LoadMaterials(const aiScene * scene)
{
	textureList.resize(scene->mNumMaterials);
	
	for (size_t i = 0; i < scene->mNumMaterials; i++)
	{
		aiMaterial* material = scene->mMaterials[i];

		textureList[i] = nullptr;

		if (material->GetTextureCount(aiTextureType_DIFFUSE))
		{
			aiString path;
			if (material->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS)
			{
				int idx = std::string(path.data).rfind("/");
				std::string filename = std::string(path.data).substr(idx + 1);

				std::string texPath = std::string("textures/") + filename;

				textureList[i] = new Texture(texPath.c_str());

				if (!textureList[i]->LoadTexture())
				{
					printf("Failed to load texture at: %s\n", texPath);
					delete textureList[i];
					textureList[i] = nullptr;
				}				
			}
		}

		if (!textureList[i])
		{
			textureList[i] = new Texture("textures/plain.png");
			textureList[i]->LoadTextureA();
		}
	}
}

void Model::ClearModel()
{
	for (size_t i = 0; i < meshList.size(); i++)
	{
		if (meshList[i])
		{
			delete meshList[i];
			meshList[i] = nullptr;
		}
	}

	for (size_t i = 0; i < textureList.size(); i++)
	{
		if (textureList[i])
		{
			delete textureList[i];
			textureList[i] = nullptr;
		}
	}
}

Model::~Model()
{
}
