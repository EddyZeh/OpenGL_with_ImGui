#pragma once
#ifndef CUBEMAP_HP
#define CUBMEMAP_HP

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "stb_image.h"

#include <string>
#include <vector>

#include "Shader.h"
#include "../memory/glMemory.hpp"
#include "MeshData.h"

class Cubemap {
public:
	Cubemap();

	void loadTextures(std::string _dir,
		std::string right   =  "right.jpg",
		std::string left    =  "left.jpg",
		std::string top     =  "top.jpg",
		std::string bottom  =  "bottom.jpg",
		std::string front   =  "front.jpg",
		std::string back    =  "back.jpg");

	void init();

	void render(Shader& shader, glm::mat4 _view, glm::mat4 projection);

	void cleanup();

private:
	unsigned int id;
	std::string dir;
	std::vector<std::string> faces;
	bool hasTextures;

	ArrayObject VAO;
};

#endif // !CUBEMAP_HP

