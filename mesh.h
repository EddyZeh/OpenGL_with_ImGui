#pragma once

#ifndef MESH_H
#define MESH_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

#include "Shader.h"
#include "Texture.h"

#include "glMemory.hpp"

struct Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoord;

	static std::vector<struct Vertex> genList(float* vertices, int noVertices);
};

class Mesh {
public:
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;

	Mesh( ){ }
	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures = {});

	void render(Shader& shader);

	ArrayObject VAO;

private:
	void setupMesh();
};

#endif // !MESH_H
