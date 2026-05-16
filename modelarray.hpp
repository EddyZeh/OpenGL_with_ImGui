#ifndef MODELARRAY_HPP
#define MODELARRAY_HPP

#include "model.h"

#define UPPER_BOUND 100

template <class T>
class ModelArray {
public:
	void init() {
		model->init();

		posVBO = BufferObject(GL_ARRAY_BUFFER);
		posVBO.bind();
		posVBO.setData<glm::vec3>(UPPER_BOUND, NULL, GL_DYNAMIC_DRAW);

		sizeVBO = BufferObject(GL_ARRAY_BUFFER);
		sizeVBO.bind();
		sizeVBO.setData<glm::vec3>(UPPER_BOUND, NULL, GL_DYNAMIC_DRAW);

		for (int i = 0; i < model->meshes.size(); i++) {
			model->meshes[i].VAO.bind();
			// set Attrib pointers
			posVBO.bind();
			posVBO.setAttPointer<glm::vec3>(3, 3, GL_FLOAT, 1, 0, 1);

			sizeVBO.bind();
			sizeVBO.setAttPointer<glm::vec3>(4, 3, GL_FLOAT, 1, 0, 1);

			ArrayObject::clear();
		}
	}

	void render(Shader& shader, bool setList = true) {
		if (setList) {

		}

		model->render(shader);

		int instanceCount = std::min(UPPER_BOUND, (int)positions.size()); // if more than 100 render only 100
		if (instanceCount > 0) {
			// if instance exist
			// reset VBOs
			posVBO.bind();
			posVBO.updateData<glm::vec3>(0, instanceCount, &positions[0]);

			sizeVBO.bind();
			sizeVBO.updateData<glm::vec3>(0, instanceCount, &sizes[0]);

			positions.clear();
			sizes.clear();
		}

		// render instanced data
		for (unsigned int i = 0; i < model->meshes.size(); i++) {
			model->meshes[i].VAO.bind();
			model->meshes[i].VAO.draw(GL_TRIANGLES, model->meshes[i].indices.size(), GL_UNSIGNED_INT, 0, instanceCount);
		}
	}

	void cleanup() {
		model->cleanup();
		posVBO.cleanup();
		sizeVBO.cleanup();
	}
	
	~ModelArray() {
		delete model;
	}

protected:
	T *model;

	BufferObject posVBO;
	BufferObject sizeVBO;

	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> sizes;
};

#endif // !MODELARRAY_HPP
