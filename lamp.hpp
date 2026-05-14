#ifndef LAMP_HPP
#define LAMP_HPP

#include "cube.hpp"

class Lamp : public Cube {
public:
	Lamp(glm::vec3 pos = glm::vec3(0.0f), glm::vec3 size = glm::vec3(1.0f), Material mat = Material::white_plastic)
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
		
		VAO.bind();
		VAO.draw(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	}
};

#endif // !LAMP_HPP
