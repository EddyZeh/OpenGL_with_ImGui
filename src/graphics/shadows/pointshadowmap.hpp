#ifndef POINT_SHADOW_MAP_HPP
#define POINT_SHADOW_MAP_HPP

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <iostream>
#include <vector>
#include <string>

#include "../framebuffer.hpp"
#include "../Shader.h"

class PointShadowMap {
public:
	void init(unsigned int width = 1024, unsigned int height = 1024) {
		this->width = width;
		this->height = height;

		glGenTextures(1, &depthCubemap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
		for (unsigned int face = 0; face < 6; face++) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0,
				GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "Point shadow map framebuffer not complete!" << std::endl;

		FramebufferObject::bindDefault();
	}

	std::vector<glm::mat4> getShadowTransforms(glm::vec3 lightPos, float nearPlane, float farPlane) const {
		float aspect = (float)width / (float)height;
		glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), aspect, nearPlane, farPlane);

		std::vector<glm::mat4> transforms;
		transforms.reserve(6);
		transforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		transforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		transforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
		transforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
		transforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		transforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		return transforms;
	}

	void beginPass(const std::vector<glm::mat4>& shadowTransforms, glm::vec3 lightPos, float farPlane, Shader& depthShader) {
		glViewport(0, 0, width, height);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glClear(GL_DEPTH_BUFFER_BIT);

		depthShader.activate();
		for (unsigned int face = 0; face < 6; face++) {
			depthShader.setMat4("shadowMatrices[" + std::to_string(face) + "]", shadowTransforms[face]);
		}
		depthShader.setFloat("far_plane", farPlane);
		depthShader.set3Float("lightPos", lightPos);
	}

	static void endPass() {
		FramebufferObject::bindDefault();
	}

	void bindForSampling(int textureUnit) const {
		glActiveTexture(GL_TEXTURE0 + textureUnit);
		glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
	}

	void cleanup() {
		glDeleteTextures(1, &depthCubemap);
		glDeleteFramebuffers(1, &fbo);
	}

	unsigned int getWidth() const { return width; }
	unsigned int getHeight() const { return height; }

private:
	GLuint fbo = 0;
	GLuint depthCubemap = 0;

	unsigned int width = 0;
	unsigned int height = 0;

};

#endif // !POINT_SHADOW_MAP_HPP
