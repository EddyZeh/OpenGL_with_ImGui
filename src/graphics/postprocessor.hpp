#ifndef POSTPROCESSOR_HPP
#define POSTPROCESSOR_HPP

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "framebuffer.hpp"
#include "Shader.h"
#include "MeshData.h"

#include "../memory/glMemory.hpp"
#include "../io/Keyboard.h"


enum class PostProcessEffect {
	NONE = 0,
	INVERT,
	GRAYSCALE,
	SHARPEN,
	EDGE,
	SOBEL,
	EMBOSS
};

class PostProcessor {
public:
	PostProcessEffect currentEffect = PostProcessEffect::NONE;

	void init(unsigned int width, unsigned int height) {
		this->width = width;
		this->height = height;

		fbo = FramebufferObject(width, height, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		fbo.generate();
		fbo.bind();
		fbo.allocateAndAttachTexture(GL_COLOR_ATTACHMENT0, GL_RGB, GL_UNSIGNED_BYTE);
		fbo.attachRBO(GL_DEPTH_STENCIL_ATTACHMENT, GL_DEPTH24_STENCIL8);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "Post-processing framebuffer not complete!" << std::endl;

		FramebufferObject::bindDefault();

		quadVAO.generate();
		quadVAO["VBO"] = BufferObject(GL_ARRAY_BUFFER);
		quadVAO.bind();
		quadVAO["VBO"].bind();
		quadVAO["VBO"].setData<float>((GLuint)MeshData::Quad.vertices.size(), MeshData::Quad.vertices.data(), GL_STATIC_DRAW);
		quadVAO["VBO"].setAttPointer<float>(0, 2, GL_FLOAT, 4, 0);
		quadVAO["VBO"].setAttPointer<float>(1, 2, GL_FLOAT, 4, 2);
		ArrayObject::clear();
	}

	void beginSceneCapture() {
		fbo.bind();
		fbo.setViewport();
	}

	void render(Shader& fboShader) {
		FramebufferObject::bindDefault();
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);
		glDisable(GL_DEPTH_TEST);

		fboShader.activate();
		quadVAO.bind();

		for (unsigned int i = 0; i < fbo.textures.size(); i++) {
			glActiveTexture(GL_TEXTURE0 + i);
			fbo.textures[i].bind();
		}

		fboShader.setInt("screenTexture", 0);
		fboShader.setFloat("screenWidth", static_cast<float>(width));
		fboShader.setFloat("screenHeight", static_cast<float>(height));
		fboShader.setInt("effect", static_cast<int>(currentEffect));

		glDrawArrays(GL_TRIANGLES, 0, 6);
		ArrayObject::clear();
	}

	void handleInput() {
		if (Keyboard::keyWentDown(GLFW_KEY_0)) currentEffect = PostProcessEffect::NONE;
		if (Keyboard::keyWentDown(GLFW_KEY_1)) currentEffect = PostProcessEffect::INVERT;
		if (Keyboard::keyWentDown(GLFW_KEY_2)) currentEffect = PostProcessEffect::GRAYSCALE;
		if (Keyboard::keyWentDown(GLFW_KEY_3)) currentEffect = PostProcessEffect::SHARPEN;
		if (Keyboard::keyWentDown(GLFW_KEY_4)) currentEffect = PostProcessEffect::EDGE;
		if (Keyboard::keyWentDown(GLFW_KEY_5)) currentEffect = PostProcessEffect::SOBEL;
		if (Keyboard::keyWentDown(GLFW_KEY_6)) currentEffect = PostProcessEffect::EMBOSS;
	}

	void cleanup() {
		fbo.cleanup();
		quadVAO.cleanup();
	}

private:
	FramebufferObject fbo;
	ArrayObject quadVAO;

	unsigned int width = 0;
	unsigned int height = 0;
};

#endif // !POSTPROCESSOR_HPP
