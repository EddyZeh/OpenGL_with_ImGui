#pragma once

#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <assimp/scene.h>

#include <string>

class Texture {
public:
	Texture();
	Texture(std::string dir, std::string path, aiTextureType type = aiTextureType_NONE, bool gammaCorrection = false);

	void generate();
	void load(bool flip = true);

	void allocate(GLenum format, GLuint width, GLuint height, GLenum type);
	
	static void setParams(GLenum texMinFilter = GL_LINEAR, 
						  GLenum texMagFilter = GL_LINEAR,
					      GLenum wrapS = GL_REPEAT,
						  GLenum wrapT = GL_REPEAT);

	unsigned int id;
	std::string dir;
	std::string path;
	aiTextureType type;

	bool gammaCorrection;

	void bind();

	void cleanup();
};

#endif // !TEXTURE_H
