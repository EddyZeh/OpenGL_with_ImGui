#ifndef PLANAR_SHADOW_MAP_HPP
#define PLANAR_SHADOW_MAP_HPP

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <iostream>

#include "../framebuffer.hpp"
#include "../Shader.h"

class PlanarShadowMap {
public:
	void init(unsigned int width = 1024, unsigned int height = 1024) {
		this->width = width;
		this->height = height;

		fbo = FramebufferObject(width, height, GL_DEPTH_BUFFER_BIT);
		fbo.generate();
		fbo.bind();

		// depth-only attachment, no color buffer
		fbo.allocateAndAttachTexture(GL_DEPTH_ATTACHMENT, GL_DEPTH_COMPONENT, GL_FLOAT);

		// clamp to a white border so anything outside	the light's frustrum reads as lit
		glBindTexture(GL_TEXTURE_2D, fbo.textures.back().id);
		Texture::setParams(GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER);
		float border[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);

		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "Planar shadow map framebuffer not complete!" << std::endl;

		FramebufferObject::bindDefault();
	}

	void beginPass(const glm::mat4& lightSpaceMatrix, Shader& depthShader) {
		fbo.activate();

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		depthShader.activate();
		depthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
	}

	static void endPass() {
		glDisable(GL_CULL_FACE);
		FramebufferObject::bindDefault();
	}

	void bindForSampling(int textureUnit) const {
		glActiveTexture(GL_TEXTURE0 + textureUnit);
		glBindTexture(GL_TEXTURE_2D, fbo.textures.front().id);
	}

	void cleanup() {
		fbo.cleanup();
	}

	unsigned int getWidth() const { return width;  }
	unsigned int getHeight() const { return height; }


private:
	FramebufferObject fbo;

	unsigned int width = 0;
	unsigned int height = 0;
};

#endif // !PLANAR_SHADOW_MAP_HPP
