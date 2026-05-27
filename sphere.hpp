#ifndef SPHERE_HPP
#define SPHERE_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

#include "Shader.h"
#include "Texture.h"
#include "material.h"

#include "modelarray.hpp"

struct SphereInstance{
	glm::vec3 position;
	glm::vec3 size;
};

class Sphere : public Model {
public:
	Material material;
	std::vector<Texture> textures;

	Sphere(glm::vec3 pos = glm::vec3(0.0f), glm::vec3 size = glm::vec3(1.0), Material material = Material::bronze)
		: Model(pos, size), material(material), textures({}), noTex(true){}

	Sphere(glm::vec3 pos = glm::vec3(0.0f), glm::vec3 size = glm::vec3(1.0f), std::vector<Texture> textures = {})
			: Model(pos, size), textures(textures), material(Material::bronze), noTex(false){}

	void init() {
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;

		int Y_SEGMENTS = 64, X_SEGMENTS = 64;

		for (unsigned int y = 0; y <= Y_SEGMENTS; y++) {
			for (unsigned int x = 0; x <= X_SEGMENTS; x++) {

				float xSegment = (float)x / (float)X_SEGMENTS;
				float ySegment = (float)y/ (float)Y_SEGMENTS;

				float xPos = std::cos(xSegment * glm::two_pi<float>()) *
						     std::sin(ySegment * glm::pi<float>());
				float yPos = std::cos(ySegment * glm::pi<float>());
				float zPos = std::sin(xSegment * glm::two_pi<float>()) *
							 std::sin(ySegment * glm::pi<float>());

				vertices.push_back({
					glm::vec3(xPos, yPos, zPos),
					glm::vec3(xPos, yPos, zPos),
					glm::vec2(xSegment, 1.0f - ySegment)
					});
			}
		}

		bool oddRow = false;
		for (unsigned int y = 0; y <= Y_SEGMENTS; y++) {
			for (unsigned int x = 0; x <= X_SEGMENTS; x++) {
				unsigned int a = (y) * (X_SEGMENTS + 1) + x;
				unsigned int b = (y + 1) * (X_SEGMENTS + 1) + x;

				indices.push_back(a);
				indices.push_back(b);
				indices.push_back(a + 1);

				indices.push_back(b);
				indices.push_back(b + 1);
				indices.push_back(a + 1);

			}
		}
		
		meshes.push_back(Mesh(vertices, indices));
	}

	virtual void render(Shader& shader) {
		shader.activate();
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, pos);
		model = glm::scale(model, size);
		shader.setMat4("model", model);
		shader.set3Float("pointLight.ambient", Material::white_plastic.ambient);
		shader.set3Float("pointLight.diffuse", Material::white_plastic.diffuse);
		shader.set3Float("pointLight.specular", Material::white_plastic.specular);

		// material properties
		if (noTex) {
			shader.setBool("noTex", noTex);
			shader.set3Float("material.diffuseColor", material.diffuse);
			shader.set3Float("material.specularColor", material.specular);
		}
		else {
			shader.setFloat("material.shininess", 64);
			shader.setInt("material.diffuseTexture", 0);
			shader.setInt("material.specularTexture", 1);
			for (unsigned int i = 0; i < textures.size(); i++) {
				glActiveTexture(GL_TEXTURE0 + i);
				textures[i].bind();
			}
		}
	}

	void cleanup() {
		meshes[0].VAO.cleanup();
	}

private:
	bool noTex;
};


class SphereArray : public ModelArray<Sphere> {
public:
	std::vector<SphereInstance> sphereInstances;

	void init(std::vector<Texture> textures) {
		model = new Sphere(glm::vec3(0.0f), glm::vec3(1.0f), std::vector<Texture>());
		ModelArray::init();
	}

	void init(Material material) {
		model = new Sphere(glm::vec3(0.0f), glm::vec3(1.0f), material);
		ModelArray::init();
	}

	void render(Shader& shader) {
		positions.clear();
		sizes.clear();

		for (auto& SI : sphereInstances) {
			positions.push_back(SI.position);
			sizes.push_back(SI.size);
		}

		ModelArray::render(shader);
	}
};

#endif // !SPHERE_HPP
