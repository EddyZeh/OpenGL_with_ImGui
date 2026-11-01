#ifndef ESCAPE_PROP_H
#define ESCAPE_PROP_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>

/*
	A Prop is one placeholder primitive in the room (a desk leg, a wall, a light bulb...).

	Props are grouped by name ("desk", "cabinet", "exit_door", ...). A group can later be
	swapped for an imported model with Level::replaceWithModel(); the primitives in that
	group are then hidden while their colliders and interaction volumes stay the same.
*/

enum class Shape {
	Cube,		// unit cube centred on the origin
	Sphere,		// diameter 1, centred on the origin
	Cylinder	// diameter 1, height 1, centred on the origin, axis = Y
};

enum class TextureId {
	None = -1,
	Brick = 0,
	Wood,
	Metal,		// container2 (+ its specular map)
	Concrete,	// bricks2
	Count
};

struct Surface {
	glm::vec3 diffuse = glm::vec3(0.6f);
	glm::vec3 specular = glm::vec3(0.2f);
	float shininess = 32.0f;

	TextureId texture = TextureId::None;
	bool worldUV = true;			// project the texture in world space so it tiles instead of stretching
	glm::vec2 uvScale = glm::vec2(1.0f);

	glm::vec3 emissive = glm::vec3(0.0f);	// glows regardless of lighting

	static Surface colour(glm::vec3 c, float spec = 0.15f, float shininess = 24.0f) {
		Surface s;
		s.diffuse = c;
		s.specular = glm::vec3(spec);
		s.shininess = shininess;
		return s;
	}

	static Surface textured(TextureId id, glm::vec2 uvScale = glm::vec2(1.0f), glm::vec3 tint = glm::vec3(1.0f)) {
		Surface s;
		s.texture = id;
		s.uvScale = uvScale;
		s.diffuse = tint;			// multiplies the texture
		s.specular = glm::vec3(1.0f);	// multiplies the specular map
		return s;
	}

	static Surface glow(glm::vec3 c, float strength = 1.0f) {
		Surface s;
		s.diffuse = c * 0.2f;
		s.specular = glm::vec3(0.0f);
		s.emissive = c * strength;
		return s;
	}
};

struct Prop {
	std::string group;
	Shape shape = Shape::Cube;
	glm::mat4 transform = glm::mat4(1.0f);
	Surface surface;

	bool visible = true;
	bool castsShadow = true;
};

// helpers for building transforms
namespace Xform {
	inline glm::mat4 box(const glm::vec3& center, const glm::vec3& size) {
		return glm::scale(glm::translate(glm::mat4(1.0f), center), size);
	}

	// box given by its min / max corners
	inline glm::mat4 span(const glm::vec3& lo, const glm::vec3& hi) {
		return box((lo + hi) * 0.5f, hi - lo);
	}

	inline glm::mat4 boxRotY(const glm::vec3& center, const glm::vec3& size, float degrees) {
		glm::mat4 m = glm::translate(glm::mat4(1.0f), center);
		m = glm::rotate(m, glm::radians(degrees), glm::vec3(0.0f, 1.0f, 0.0f));
		return glm::scale(m, size);
	}

	// cylinder lying along X or Z (default cylinder axis is Y)
	inline glm::mat4 cylinderAlong(const glm::vec3& center, float length, float diameter, char axis) {
		glm::mat4 m = glm::translate(glm::mat4(1.0f), center);
		if (axis == 'x')
			m = glm::rotate(m, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		else if (axis == 'z')
			m = glm::rotate(m, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		return glm::scale(m, glm::vec3(diameter, length, diameter));
	}
}

#endif // !ESCAPE_PROP_H
