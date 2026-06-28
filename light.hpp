#ifndef LIGHT_HPP
#define LIGHT_HPP

#include <glm/glm.hpp>

#include "Shader.h"
#include "material.h"

struct DirLight {
	glm::vec3 direction;
	
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;

	void render(Shader& shader) {
		std::string name = "dirlight";
		shader.activate();
		shader.set3Float(name + ".direction", direction);
		shader.set3Float(name + ".ambient", ambient);
		shader.set3Float(name + ".diffuse", diffuse);
		shader.set3Float(name + ".specular", specular);
	}
};

struct PointLight {
	glm::vec3 position;

	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;

	float k0, k1, k2;

	void render(Shader& shader, int idx) {
		std::string name = "pointLights[" + std::to_string(idx) + "]";
		shader.activate();

		shader.set3Float(name + ".position",  position);
		shader.set3Float(name + ".ambient",   ambient);
		shader.set3Float(name + ".diffuse",   diffuse);
		shader.setFloat( name + ".constant",  k0);
		shader.setFloat( name + ".linear",    k1);
		shader.setFloat( name + ".quadratic", k2);
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

	void render(Shader& shader, int idx) {
		std::string name = "spotLights[" + std::to_string(idx) + "]";
		shader.activate();

		shader.set3Float(name + ".position", position);
		shader.set3Float(name + ".direction", direction);

		shader.setFloat(name + ".cutOff", cutOff);
		shader.setFloat(name + ".outerCutOff", outerCutOff);

		shader.setFloat(name + ".constant", k0);
		shader.setFloat(name + ".linear", k1);
		shader.setFloat(name + ".quadratic", k2);

		shader.set3Float(name + ".ambient", ambient);
		shader.set3Float(name + ".diffuse", diffuse);
		shader.set3Float(name + ".specular", specular);
	}
};

#endif // !LIGHT_HPP
