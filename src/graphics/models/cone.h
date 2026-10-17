#pragma once

#ifndef CONE_HPP
#define CONE_HPP

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <vector>

#include "../../memory/glMemory.hpp"

class ConeMesh {
public:
	ConeMesh(float length, float halfAngleRadius, unsigned int segments = 48);

	~ConeMesh();

	// Avoid accidental copy of GPU resource
	ConeMesh(const ConeMesh&) = delete;
	ConeMesh& operator=(const ConeMesh&) = delete;

	void draw();

	float getLength() { return length; }
	float getRadius() { return radius; }

private:
	void generateMesh();

	void uploadMesh(std::vector<glm::vec3> &vertices, std::vector<std::uint32_t> & indices);

	float length;
	float radius;
	float halfAngleRaius;
	unsigned int segments;

	GLsizei indexCount = 0;
	ArrayObject VAO;

};

#endif // !CONE_HPP
