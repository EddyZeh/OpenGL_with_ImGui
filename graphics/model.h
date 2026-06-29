#pragma once

#ifndef MODEL_H
#define MODEL_H

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include "mesh.h"
#include "../memory/ResourceManager.h"

class Model {
public:
	glm::vec3 pos;
	glm::vec3 size;
	bool noTex;
	Model(glm::vec3 pos = glm::vec3(0.0f), glm::vec3 size = glm::vec3(1.0f), bool noTex = false);

	virtual void render(Shader& shader);
	std::vector<Mesh> meshes;
	std::vector<Texture> textures_loaded;
	void loadModel(std::string path);

private:
	std::string directory;
	
	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type);
};

#endif // !MODEL_H
