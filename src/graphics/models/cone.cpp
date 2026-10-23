#include "cone.h"

#include <glm/gtc/constants.hpp>

#include <cmath>
#include <cstdint>
#include <stdexcept>

ConeMesh::ConeMesh(float length, float halfAngleRadius, unsigned int segments, bool capBase)
	: length(length), halfAngleRaius(halfAngleRadius), segments(segments), capBase(capBase) {

	if (length <= 0.0 || halfAngleRadius <= 0.0f ||
		halfAngleRadius >= glm::half_pi<float>() || segments < 3) {

		throw std::invalid_argument("Invalid cone dimensions or segements count");
	}

	radius = length * glm::tan(halfAngleRadius);

	generateMesh();
}

ConeMesh::~ConeMesh() {
	VAO.cleanup();
}

void ConeMesh::draw() {
	VAO.bind();
	VAO.draw(GL_TRIANGLES, static_cast<GLuint>(indexCount), GL_UNSIGNED_INT, 0);
	ArrayObject::clear();
}

void ConeMesh::generateMesh() {
	std::vector<glm::vec3> vertices;
	std::vector<std::uint32_t> indices;

	vertices.reserve(segments + 2);
	indices.reserve(segments * 6);

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

	// Same winding rule: counter-clockwise seen from outside (+Z), so the normal points away from the apex
	if (capBase) {
		const unsigned int centre = static_cast<unsigned int>(vertices.size());
		vertices.emplace_back(0.0f, 0.0f, length);

		for (unsigned int i = 0; i < segments; i++) {
			const unsigned int curr = i + 1;
			const unsigned int next = (i + 1) % segments + 1;

			indices.push_back(centre);
			indices.push_back(curr);
			indices.push_back(next);
		}
	}

	indexCount = static_cast<GLsizei>(indices.size());
	uploadMesh(vertices, indices);
}

void ConeMesh::uploadMesh(std::vector<glm::vec3>& vertices, std::vector<std::uint32_t>& indices) {
	VAO.generate();
	VAO["VBO"] = BufferObject(GL_ARRAY_BUFFER);
	VAO["EBO"] = BufferObject(GL_ELEMENT_ARRAY_BUFFER);
	VAO.bind();
	VAO["VBO"].bind();
	VAO["VBO"].setData<glm::vec3>(static_cast<GLuint>(vertices.size()), vertices.data(), GL_STATIC_DRAW);

	VAO["EBO"].bind();
	VAO["EBO"].setData<std::uint32_t>(static_cast<GLuint>(indices.size()), indices.data(), GL_STATIC_DRAW);

	VAO["VBO"].setAttPointer<float>(0, 3, GL_FLOAT, 3, 0);

	ArrayObject::clear();
}