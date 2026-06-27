#ifndef LAMP_HPP
#define LAMP_HPP

#include "cube.hpp"

class Lamp : public Cube {
public:
	Lamp(glm::vec3 pos = glm::vec3(0.0f), glm::vec3 size = glm::vec3(0.5f), Material mat = Material::white_plastic)
		: Cube(pos, size, mat){	}

	void render(Shader& shader) override {
		shader.activate();
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, pos);
		model = glm::scale(model, size);
		shader.setMat4("model", model);
		shader.set3Float("material.ambient", material.ambient);
		shader.set3Float("mateiral.diffuse", material.diffuse);
		shader.set3Float("material.specular", material.specular);
		shader.setFloat("material.shininess", material.shininess);		
	}
};


class LampArray : public ModelArray<Lamp>{


public:
	std::vector<glm::vec3> pointLightPos;

	void init() {
		model = new Lamp();
		ModelArray::init();
	}

	void render(Shader& shader) {
		positions.clear();
		sizes.clear();

		for (auto pl : pointLightPos) {
			positions.push_back(pl);
			sizes.push_back(model->size);
		}

		ModelArray::render(shader);
	}
};

#endif // !LAMP_HPP
