#ifndef LIGHTCONE_HPP
#define LIGHTCONE_HPP

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cmath>
#include <memory>

#include "Shader.h"
#include "light.hpp"
#include "models/cone.h"


class LightCone {
public:

	glm::vec3 tint = glm::vec3(1.0f);
	float density = 0.15f;		
	float intensity = 1.5f;		
	float anisotropy = 0.35f;	
	float shadowBias = 0.002f;	
	int   steps = 48;			

	
	void init(const SpotLight& light, float beamLength = 15.0f) {
		length = beamLength;
		build(light.outerCutOff);
	}

	void render(Shader& shader, const SpotLight& light,
		const glm::mat4& view, const glm::mat4& projection, const glm::vec3& viewPos,
		bool useShadows = true, int textureUnit = LightUnits::SPOT_DEPTH0) {

		if (!cone || light.outerCutOff != builtForCutOff)
			build(light.outerCutOff);

		const glm::vec3 dir = glm::normalize(light.direction);

		shader.activate();

		shader.setMat4("model", modelFor(light.position, dir));
		shader.setMat4("view", view);
		shader.setMat4("projection", projection);
		shader.set3Float("viewPos", viewPos);

		shader.set3Float("lightPos", light.position);
		shader.set3Float("lightDir", dir);
		shader.setFloat("cutOff", light.cutOff);
		shader.setFloat("outerCutOff", light.outerCutOff);
		shader.setFloat("k0", light.k0);
		shader.setFloat("k1", light.k1);
		shader.setFloat("k2", light.k2);
		shader.set3Float("lightColor", light.diffuse * tint);

		shader.setMat4("lightSpaceMatrix", light.getLightSpaceMatrix());
		shader.setFloat("shadowBias", shadowBias);
		shader.setBool("useShadows", useShadows);
		light.shadowMap.bindForSampling(textureUnit);
		shader.setInt("shadowMap", textureUnit);

		shader.setFloat("maxChord", slant);
		shader.setFloat("density", density);
		shader.setFloat("anisotropy", anisotropy);
		shader.setFloat("intensity", intensity);
		shader.setInt("steps", (std::max)(steps, 1));

		const GLboolean blendWas = glIsEnabled(GL_BLEND);
		const GLboolean depthTestWas = glIsEnabled(GL_DEPTH_TEST);
		const GLboolean cullWas = glIsEnabled(GL_CULL_FACE);
		GLint cullModeWas, blendSrcWas, blendDstWas;
		GLboolean depthMaskWas;
		glGetIntegerv(GL_CULL_FACE_MODE, &cullModeWas);
		glGetIntegerv(GL_BLEND_SRC_RGB, &blendSrcWas);
		glGetIntegerv(GL_BLEND_DST_RGB, &blendDstWas);
		glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMaskWas);

		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);

		// back faces only, so each pixel is shaded once at the far side of the volume
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);

		cone->draw();

		glCullFace(static_cast<GLenum>(cullModeWas));
		if (!cullWas) glDisable(GL_CULL_FACE);
		glDepthMask(depthMaskWas);
		if (depthTestWas) glEnable(GL_DEPTH_TEST);
		glBlendFunc(static_cast<GLenum>(blendSrcWas), static_cast<GLenum>(blendDstWas));
		if (!blendWas) glDisable(GL_BLEND);

		glActiveTexture(GL_TEXTURE0);
	}

	void cleanup() {
		cone.reset();
	}

private:
	void build(float outerCutOff) {
		const float halfAngle = std::acos(glm::clamp(outerCutOff, 0.01f, 0.9999f));
		cone = std::make_unique<ConeMesh>(length, halfAngle);
		builtForCutOff = outerCutOff;
		slant = length / std::cos(halfAngle);
		slant = (std::max)(slant, 2.0f * cone->getRadius());
	}

	static glm::mat4 modelFor(const glm::vec3& position, const glm::vec3& dir) {
		const glm::vec3 up = LightUnits::upFor(dir);
		const glm::vec3 x = glm::normalize(glm::cross(up, dir));
		const glm::vec3 y = glm::cross(dir, x);

		return glm::mat4(glm::vec4(x, 0.0f),
			glm::vec4(y, 0.0f),
			glm::vec4(dir, 0.0f),
			glm::vec4(position, 1.0f));
	}

	std::unique_ptr<ConeMesh> cone;
	float length = 15.0f;
	float slant = 15.0f;
	float builtForCutOff = -2.0f;
};

#endif // !LIGHTCONE_HPP
