#include "Primitives.h"

#include <glm/gtc/constants.hpp>

#include <cmath>
#include <vector>

void Primitives::init() {
	cube = buildCube();
	sphere = buildSphere(24);
	cylinder = buildCylinder(24);
}

void Primitives::draw(Shape shape) {
	Mesh* m = &cube;
	if (shape == Shape::Sphere)
		m = &sphere;
	else if (shape == Shape::Cylinder)
		m = &cylinder;

	m->VAO.bind();
	m->VAO.draw(GL_TRIANGLES, (GLuint)m->indices.size(), GL_UNSIGNED_INT, 0);
	ArrayObject::clear();
}

void Primitives::cleanup() {
	cube.cleanup();
	sphere.cleanup();
	cylinder.cleanup();
}

Mesh Primitives::buildCube() {
	std::vector<Vertex> v;
	std::vector<unsigned int> idx;

	// normal, u axis, v axis (u x v = normal => CCW from outside)
	const glm::vec3 faces[6][3] = {
		{ { 1, 0, 0 }, { 0, 0, -1 }, { 0, 1, 0 } },
		{ { -1, 0, 0 }, { 0, 0, 1 }, { 0, 1, 0 } },
		{ { 0, 1, 0 }, { 1, 0, 0 }, { 0, 0, -1 } },
		{ { 0, -1, 0 }, { 1, 0, 0 }, { 0, 0, 1 } },
		{ { 0, 0, 1 }, { 1, 0, 0 }, { 0, 1, 0 } },
		{ { 0, 0, -1 }, { -1, 0, 0 }, { 0, 1, 0 } }
	};
	const glm::vec2 corners[4] = { { -1, -1 }, { 1, -1 }, { 1, 1 }, { -1, 1 } };

	for (const auto& f : faces) {
		unsigned int base = (unsigned int)v.size();
		for (const auto& c : corners) {
			Vertex vert;
			vert.Position = f[0] * 0.5f + f[1] * (c.x * 0.5f) + f[2] * (c.y * 0.5f);
			vert.Normal = f[0];
			vert.TexCoord = glm::vec2(c.x * 0.5f + 0.5f, c.y * 0.5f + 0.5f);
			v.push_back(vert);
		}
		idx.insert(idx.end(), { base, base + 1, base + 2, base + 2, base + 3, base });
	}
	return Mesh(v, idx);
}

Mesh Primitives::buildSphere(unsigned int segments) {
	std::vector<Vertex> v;
	std::vector<unsigned int> idx;
	const unsigned int rings = segments / 2;

	for (unsigned int y = 0; y <= rings; y++) {
		float phi = glm::pi<float>() * (float)y / (float)rings;
		for (unsigned int x = 0; x <= segments; x++) {
			float theta = glm::two_pi<float>() * (float)x / (float)segments;
			glm::vec3 n(std::cos(theta) * std::sin(phi), std::cos(phi), std::sin(theta) * std::sin(phi));
			Vertex vert;
			vert.Position = n * 0.5f;
			vert.Normal = n;
			vert.TexCoord = glm::vec2((float)x / segments, 1.0f - (float)y / rings);
			v.push_back(vert);
		}
	}

	const unsigned int row = segments + 1;
	for (unsigned int y = 0; y < rings; y++) {
		for (unsigned int x = 0; x < segments; x++) {
			unsigned int a = y * row + x;
			unsigned int b = (y + 1) * row + x;
			unsigned int c = (y + 1) * row + x + 1;
			unsigned int d = y * row + x + 1;
			idx.insert(idx.end(), { a, c, b, a, d, c });
		}
	}
	return Mesh(v, idx);
}

Mesh Primitives::buildCylinder(unsigned int segments) {
	std::vector<Vertex> v;
	std::vector<unsigned int> idx;

	// side
	for (unsigned int i = 0; i <= segments; i++) {
		float t = glm::two_pi<float>() * (float)i / (float)segments;
		glm::vec3 n(std::cos(t), 0.0f, std::sin(t));
		Vertex bottom, top;
		bottom.Position = n * 0.5f + glm::vec3(0.0f, -0.5f, 0.0f);
		top.Position = n * 0.5f + glm::vec3(0.0f, 0.5f, 0.0f);
		bottom.Normal = top.Normal = n;
		bottom.TexCoord = glm::vec2((float)i / segments, 0.0f);
		top.TexCoord = glm::vec2((float)i / segments, 1.0f);
		v.push_back(bottom);
		v.push_back(top);
	}
	for (unsigned int i = 0; i < segments; i++) {
		unsigned int b0 = i * 2, t0 = i * 2 + 1, b1 = i * 2 + 2, t1 = i * 2 + 3;
		idx.insert(idx.end(), { b0, t0, t1, b0, t1, b1 });
	}

	// caps
	for (int cap = 0; cap < 2; cap++) {
		float y = cap == 0 ? 0.5f : -0.5f;
		glm::vec3 n(0.0f, cap == 0 ? 1.0f : -1.0f, 0.0f);

		unsigned int centre = (unsigned int)v.size();
		Vertex c;
		c.Position = glm::vec3(0.0f, y, 0.0f);
		c.Normal = n;
		c.TexCoord = glm::vec2(0.5f);
		v.push_back(c);

		for (unsigned int i = 0; i <= segments; i++) {
			float t = glm::two_pi<float>() * (float)i / (float)segments;
			Vertex r;
			r.Position = glm::vec3(std::cos(t) * 0.5f, y, std::sin(t) * 0.5f);
			r.Normal = n;
			r.TexCoord = glm::vec2(std::cos(t) * 0.5f + 0.5f, std::sin(t) * 0.5f + 0.5f);
			v.push_back(r);
		}
		for (unsigned int i = 0; i < segments; i++) {
			unsigned int r0 = centre + 1 + i, r1 = centre + 2 + i;
			if (cap == 0)
				idx.insert(idx.end(), { centre, r1, r0 });
			else
				idx.insert(idx.end(), { centre, r0, r1 });
		}
	}
	return Mesh(v, idx);
}
