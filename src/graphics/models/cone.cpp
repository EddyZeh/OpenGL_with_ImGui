#include "cone.h"

#include <glm/gtc/constants.hpp>

#include <cmath>
#include <cstdint>
#include <stdexcept>

ConeMesh::ConeMesh(float length, float halfAngleRadius, unsigned int segments)
				  : length(length), halfAngleRaius(halfAngleRadius), segments(segments){

	if (length <= 0.0 || halfAngleRadius <= 0.0f ||
		halfAngleRadius >= glm::half_pi<float>() || segments < 3) {

		throw std::invalid_argument("Invalid cone dimensions or segements count");
	}

	radius = length * glm::tan(halfAngleRadius);

	generateMesh();
}

ConeMesh::~ConeMesh(){
	VAO.cleanup();
}

void ConeMesh::draw(){
	VAO.bind();
	VAO.draw(GL_TRIANGLES, indexCount);
	ArrayObject::clear();
}

void ConeMesh::generateMesh(){
	std::vector<glm::vec3> vertices;
	std::vector<std::uint32_t> indices;

	vertices.reserve(segments + 1);
	indices.reserve(segments + 3);

	// Apex vertices (vertices[0])
	vertices.emplace_back(0.0, 0.0, 0.0);

	const float angleStep = glm::two_pi<float>() / static_cast<float>(segments);

	// Generate circular base vertices
	for (unsigned int i = 0; i < segments; i++) {
		const float angle = i * angleStep;

		float x = radius * glm::cos(angle);
		float y = radius * glm::sin(angle);
		float z = length;

		vertices.emplace_back(x, y, z);
	}

	// Connect apex to each pair of neighboring vertices of the base ring
	for (unsigned int i = 0; i < segments; i++) {
		const unsigned int curr = i + 1;
		const unsigned int next = (i + 1) % segments + 1;

		// (0, Bnext, Bcurr) for counter-clockwise winding
		indices.push_back(0);
		indices.push_back(next);
		indices.push_back(curr);
	}

	indexCount = static_cast<GLsizei>(indices.size());
	uploadMesh(vertices, indices);
}

void ConeMesh::uploadMesh(std::vector<glm::vec3> &vertices, std::vector<std::uint32_t> &indices){
	VAO.generate();
	VAO["VBO"] = BufferObject(GL_ARRAY_BUFFER);
	VAO["EBO"] = BufferObject(GL_ELEMENT_ARRAY_BUFFER);
	VAO.bind();
	VAO["VBO"].bind();
	VAO["VBO"].setData<glm::vec3>(static_cast<GLuint>(vertices.size()), vertices.data(), GL_STATIC_DRAW);

	VAO["EBO"].bind();
	VAO["EBO"].setData<std::uint32_t>(static_cast<GLuint>(indices.size()), indices.data(), GL_STATIC_DRAW);

	VAO["VBO"].setAttPointer<glm::vec3>(0, 3, GL_FLOAT, 3, 0);

	ArrayObject::clear();
}