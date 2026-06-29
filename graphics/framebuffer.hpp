#ifndef FRAMEBUFFER_HPP
#define FRAMEBUFFER_HPP

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <vector>

#include "Texture.h"

class FramebufferObject {
public:
	GLuint val;
	GLuint width;
	GLuint height;
	GLbitfield bitCombo;

	std::vector<GLuint> rbos;
	std::vector<Texture> textures;

	FramebufferObject()
		: val(0), width(0), height(0), bitCombo(0) { }

	FramebufferObject(GLuint width, GLuint height, GLbitfield bitCombo)
		: val(0), width(width), height(height), bitCombo(bitCombo) { }

	void generate() {
		glGenFramebuffers(1, &val);
	}

	void bind() {
		glBindFramebuffer(GL_FRAMEBUFFER, val);
	}

	void setViewport() {
		glViewport(0, 0, width, height);
	}

	void clear() {
		glClear(bitCombo);
	}

	void activate() {
		bind();
		setViewport();
		clear();
	}

	void attachRBO(GLenum attachType, GLenum format) {
		GLuint rbo;

		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, format, width, height);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachType, GL_RENDERBUFFER, rbo);

		rbos.push_back(rbo);
	}

	void allocateAndAttachTexture(GLenum attachType, GLenum format, GLenum type) {
		Texture tex;
		tex.generate();
		tex.bind();
		tex.allocate(format, width, height, type);
		Texture::setParams();

		bind();
		glFramebufferTexture2D(GL_FRAMEBUFFER, attachType, GL_TEXTURE_2D, tex.id, 0);
		textures.push_back(tex);		
	}

	void cleanup() {
		// Delete RBO
		glDeleteRenderbuffers(static_cast<GLsizei>(rbos.size()), &rbos[0]);

		// Delete generated textures
		for (auto& tex : textures)
			tex.cleanup();

		// Delete framebuffer
		glDeleteFramebuffers(1, &val);
	}

	static void bindDefault() {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

};


#endif // !FRAMEBUFFER_HPP
