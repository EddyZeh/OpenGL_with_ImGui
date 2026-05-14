#pragma once

#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>

class Texture {
public:
	Texture();
	Texture(std::string dir, std::string path, bool gammaCorrection = false);

	void generate();
	void load(bool flip = true);

	unsigned int id;
	std::string dir;
	std::string path;

	bool gammaCorrection;

	void bind();
};

#endif // !TEXTURE_H
