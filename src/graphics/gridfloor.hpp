#ifndef GRIDFLOOR_HPP
#define GRIDFLOOR_HPP

#include <glad/glad.h>

#include <glm/glm.hpp>

#include <vector>

#include "Shader.h"
#include "../memory/glMemory.hpp"

class GridFloor {
public:
	glm::vec3 baseColor = glm::vec3(0.16f, 0.17f, 0.19f);
	glm::vec3 lineColor = glm::vec3(0.42f, 0.45f, 0.50f);
	glm::vec3 xAxisColor = glm::vec3(0.80f, 0.25f, 0.28f);
	glm::vec3 zAxisColor = glm::vec3(0.25f, 0.45f, 0.85f);

	float cellSize = 1.0f;	
	float majorEvery = 10.0f;
	float fadeDistance = 60.0f;

	void init(float extent = 150.0f) {
		this->extent = extent;

		const float e = extent;
		std::vector<float> vertices = {
			-e, 0.0f, -e,
			 e, 0.0f, -e,
			 e, 0.0f,  e,

			-e, 0.0f, -e,
			 e, 0.0f,  e,
			-e, 0.0f,  e
		};

		VAO.generate();
		VAO["VBO"] = BufferObject(GL_ARRAY_BUFFER);
		VAO.bind();
		VAO["VBO"].bind();
		VAO["VBO"].setData<float>((GLuint)vertices.size(), vertices.data(), GL_STATIC_DRAW);
		VAO["VBO"].setAttPointer<float>(0, 3, GL_FLOAT, 3, 0);
		ArrayObject::clear();
	}

	void render(Shader& shader, const glm::mat4& view, const glm::mat4& projection,
		const glm::vec3& viewPos, float height, float holeHalfExtent) {
		shader.activate();
		shader.setMat4("view", view);
		shader.setMat4("projection", projection);
		shader.set3Float("viewPos", viewPos);
		shader.setFloat("gridHeight", height);

		shader.set3Float("baseColor", baseColor);
		shader.set3Float("lineColor", lineColor);
		shader.set3Float("xAxisColor", xAxisColor);
		shader.set3Float("zAxisColor", zAxisColor);

		shader.setFloat("cellSize", cellSize);
		shader.setFloat("majorEvery", majorEvery);
		shader.setFloat("fadeDistance", fadeDistance);
		shader.setFloat("holeHalfExtent", holeHalfExtent);

		GLboolean blendWasOn = glIsEnabled(GL_BLEND);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		VAO.bind();
		glDrawArrays(GL_TRIANGLES, 0, 6);
		ArrayObject::clear();

		if (!blendWasOn)
			glDisable(GL_BLEND);
	}

	void cleanup() {
		VAO.cleanup();
	}

private:
	ArrayObject VAO;
	float extent = 0.0f;
};

#endif // !GRIDFLOOR_HPP
