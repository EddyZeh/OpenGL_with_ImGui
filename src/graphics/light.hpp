#ifndef LIGHT_HPP
#define LIGHT_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cmath>
#include <string>

#include "Shader.h"
#include "material.h"

#include "shadows/planarshadowmap.hpp"
#include "shadows/pointshadowmap.hpp"

namespace LightUnits {
	constexpr int MAX_POINT_LIGHTS = 8;
	constexpr int MAX_SPOT_LIGHTS = 4;

	constexpr int DIR_DEPTH = 2;
	constexpr int SPOT_DEPTH0 = DIR_DEPTH + 1;
	constexpr int POINT_DEPTH0 = SPOT_DEPTH0 + MAX_SPOT_LIGHTS;

	// up vector for a light's lookAt, avoiding the degenerate straight-down case
	inline glm::vec3 upFor(const glm::vec3& direction) {
		glm::vec3 dir = glm::normalize(direction);
		return (std::abs(dir.y) > 0.999f) ? glm::vec3(0.0f, 0.0f, 1.0f)
			: glm::vec3(0.0f, 1.0f, 0.0f);
	}

	inline void configureSamplers(Shader& shader) {
		shader.activate();
		shader.setInt("dirDepthMap", DIR_DEPTH);

		for (int i = 0; i < MAX_SPOT_LIGHTS; i++)
			shader.setInt("spotDepthMaps[" + std::to_string(i) + "]", SPOT_DEPTH0 + i);

		for (int i = 0; i < MAX_POINT_LIGHTS; i++)
			shader.setInt("pointDepthMaps[" + std::to_string(i) + "]", POINT_DEPTH0 + i);
	}
}

struct DirLight {
	glm::vec3 direction;

	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;

	// shadow frustum: an ortho box aimed at shadowTarget from shadowDistance away
	PlanarShadowMap shadowMap;
	glm::vec3 shadowTarget = glm::vec3(0.0f);
	float shadowDistance = 15.0f;
	float shadowOrthoSize = 12.0f;
	float shadowNear = 1.0f;
	float shadowFar = 40.0f;

	void initShadowMap(unsigned int resolution = 1024) {
		shadowMap.init(resolution, resolution);
	}

	// orthographic: a directional light is treated as infinitely far away
	glm::mat4 getLightSpaceMatrix() const {
		glm::vec3 dir = glm::normalize(direction);
		glm::mat4 lightProj = glm::ortho(-shadowOrthoSize, shadowOrthoSize,
			-shadowOrthoSize, shadowOrthoSize,
			shadowNear, shadowFar);
		glm::mat4 lightView = glm::lookAt(shadowTarget - dir * shadowDistance,
			shadowTarget,
			LightUnits::upFor(dir));
		return lightProj * lightView;
	}

	// templated on the renderable so light.hpp stays free of the model headers
	template <class Renderable>
	void renderShadowPass(Shader& depthShader, Renderable& objects) {
		shadowMap.beginPass(getLightSpaceMatrix(), depthShader);
		objects.renderShadowCasters(depthShader);
		PlanarShadowMap::endPass();
	}

	void render(Shader& shader, int textureUnit = LightUnits::DIR_DEPTH) {
		shader.activate();
		shader.setBool("hasDirLight", true);

		std::string name = "dirLight";
		shader.set3Float(name + ".direction", direction);
		shader.set3Float(name + ".ambient", ambient);
		shader.set3Float(name + ".diffuse", diffuse);
		shader.set3Float(name + ".specular", specular);

		shader.setMat4("dirLightSpaceMatrix", getLightSpaceMatrix());
		shader.setInt("dirDepthMap", textureUnit);
		shadowMap.bindForSampling(textureUnit);
	}

	void cleanup() {
		shadowMap.cleanup();
	}
};

struct PointLight {
	glm::vec3 position;

	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;

	float k0, k1, k2;

	// omnidirectional, so the depth cubemap stores distance in [0, shadowFar]
	PointShadowMap shadowMap;
	float shadowNear = 1.0f;
	float shadowFar = 25.0f;

	void initShadowMap(unsigned int resolution = 1024) {
		shadowMap.init(resolution, resolution);
	}

	template <class Renderable>
	void renderShadowPass(Shader& depthShader, Renderable& objects) {
		shadowMap.beginPass(shadowMap.getShadowTransforms(position, shadowNear, shadowFar),
			position, shadowFar, depthShader);
		objects.renderShadowCasters(depthShader);
		PointShadowMap::endPass();
	}

	void render(Shader& shader, int idx, int textureUnit = -1) {
		if (textureUnit < 0)
			textureUnit = LightUnits::POINT_DEPTH0 + idx;

		shader.activate();

		std::string name = "pointLights[" + std::to_string(idx) + "]";
		shader.set3Float(name + ".position", position);
		shader.set3Float(name + ".ambient", ambient);
		shader.set3Float(name + ".diffuse", diffuse);
		shader.set3Float(name + ".specular", specular);
		shader.setFloat(name + ".k0", k0);
		shader.setFloat(name + ".k1", k1);
		shader.setFloat(name + ".k2", k2);

		shader.setFloat("far_plane", shadowFar);

		shader.setInt("pointDepthMaps[" + std::to_string(idx) + "]", textureUnit);
		shadowMap.bindForSampling(textureUnit);
	}

	void cleanup() {
		shadowMap.cleanup();
	}
};

struct SpotLight {
	glm::vec3 position;
	glm::vec3 direction;


	float cutOff;
	float outerCutOff;

	float k0, k1, k2;

	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;

	PlanarShadowMap shadowMap;
	float shadowNear = 0.5f;
	float shadowFar = 30.0f;

	void initShadowMap(unsigned int resolution = 1024) {
		shadowMap.init(resolution, resolution);
	}

	glm::mat4 getLightSpaceMatrix() const {
		float fov = 2.0f * std::acos(glm::clamp(outerCutOff, -1.0f, 1.0f));
		fov = glm::clamp(fov, glm::radians(1.0f), glm::radians(175.0f));

		glm::vec3 dir = glm::normalize(direction);
		glm::mat4 lightProj = glm::perspective(fov, 1.0f, shadowNear, shadowFar);
		glm::mat4 lightView = glm::lookAt(position, position + dir, LightUnits::upFor(dir));
		return lightProj * lightView;
	}

	template <class Renderable>
	void renderShadowPass(Shader& depthShader, Renderable& objects) {
		shadowMap.beginPass(getLightSpaceMatrix(), depthShader);
		objects.renderShadowCasters(depthShader);
		PlanarShadowMap::endPass();
	}

	void render(Shader& shader, int idx, int textureUnit = -1) {
		if (textureUnit < 0)
			textureUnit = LightUnits::SPOT_DEPTH0 + idx;

		shader.activate();

		std::string name = "spotLights[" + std::to_string(idx) + "]";
		shader.set3Float(name + ".position", position);
		shader.set3Float(name + ".direction", direction);
		shader.setFloat(name + ".cutOff", cutOff);
		shader.setFloat(name + ".outerCutOff", outerCutOff);
		shader.setFloat(name + ".k0", k0);
		shader.setFloat(name + ".k1", k1);
		shader.setFloat(name + ".k2", k2);
		shader.set3Float(name + ".ambient", ambient);
		shader.set3Float(name + ".diffuse", diffuse);
		shader.set3Float(name + ".specular", specular);

		shader.setMat4("spotLightSpaceMatrix[" + std::to_string(idx) + "]", getLightSpaceMatrix());
		shader.setInt("spotDepthMaps[" + std::to_string(idx) + "]", textureUnit);
		shadowMap.bindForSampling(textureUnit);
	}

	void cleanup() {
		shadowMap.cleanup();
	}
};

#endif // !LIGHT_HPP
