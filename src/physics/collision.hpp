#ifndef COLLISION_HPP
#define COLLISION_HPP

/*
	Engine-level collision queries.

	Header-only and free of any game rules, so any project built on the engine can use it:
	  - AABB              axis-aligned bounding box (min / max corners)
	  - Ray               origin + normalised direction
	  - rayVsAABB         slab test, returns the entry distance
	  - transformAABB     bounds of a box after an arbitrary transform (for rotating doors etc.)
	  - penetration       smallest push that separates two overlapping boxes on the XZ plane

	Collision detection (can I move here?) and ray casting (what am I looking at?) are
	separate problems that happen to share the same box maths.
*/

#include <glm/glm.hpp>

#include <cmath>
#include <limits>

// Note: written to survive <windows.h> min/max macros (parenthesised calls, brace init).

namespace Collision {

	struct AABB {
		glm::vec3 min = glm::vec3(0.0f);
		glm::vec3 max = glm::vec3(0.0f);

		AABB() = default;
		AABB(const glm::vec3& lo, const glm::vec3& hi) : min{ lo }, max{ hi } {}

		static AABB fromCenterSize(const glm::vec3& center, const glm::vec3& size) {
			glm::vec3 half = size * 0.5f;
			return AABB(center - half, center + half);
		}

		glm::vec3 center() const { return (min + max) * 0.5f; }
		glm::vec3 size() const { return max - min; }

		bool intersects(const AABB& o) const {
			return (min.x < o.max.x && max.x > o.min.x) &&
				(min.y < o.max.y && max.y > o.min.y) &&
				(min.z < o.max.z && max.z > o.min.z);
		}

		bool contains(const glm::vec3& p) const {
			return p.x >= min.x && p.x <= max.x &&
				p.y >= min.y && p.y <= max.y &&
				p.z >= min.z && p.z <= max.z;
		}

		// grow every side by `amount` (negative shrinks)
		AABB expanded(float amount) const {
			return AABB(min - glm::vec3(amount), max + glm::vec3(amount));
		}

		// make sure no axis is thinner than `minThickness` (thin notes / cards are hard to aim at)
		AABB withMinThickness(float minThickness) const {
			AABB r = *this;
			glm::vec3 c = center();
			for (int i = 0; i < 3; i++) {
				if (r.max[i] - r.min[i] < minThickness) {
					r.min[i] = c[i] - minThickness * 0.5f;
					r.max[i] = c[i] + minThickness * 0.5f;
				}
			}
			return r;
		}
	};

	struct Ray {
		glm::vec3 origin;
		glm::vec3 direction;	// expected to be normalised
	};

	// Slab test. On a hit, tHit is the distance along the ray where it enters the box
	// (0 if the origin is already inside).
	inline bool rayVsAABB(const Ray& ray, const AABB& box, float& tHit) {
		float tMin = 0.0f;
		float tMax = (std::numeric_limits<float>::max)();

		for (int axis = 0; axis < 3; axis++) {
			float o = ray.origin[axis];
			float d = ray.direction[axis];

			if (std::abs(d) < 1e-8f) {
				// parallel to this slab: must already be between the planes
				if (o < box.min[axis] || o > box.max[axis])
					return false;
				continue;
			}

			float inv = 1.0f / d;
			float t1 = (box.min[axis] - o) * inv;
			float t2 = (box.max[axis] - o) * inv;
			if (t1 > t2) { float tmp = t1; t1 = t2; t2 = tmp; }

			if (t1 > tMin) tMin = t1;
			if (t2 < tMax) tMax = t2;
			if (tMin > tMax)
				return false;
		}

		tHit = tMin;
		return true;
	}

	// Bounds of a unit cube ([-0.5, 0.5]^3) after `transform`.
	inline AABB transformUnitCube(const glm::mat4& transform) {
		glm::vec3 lo((std::numeric_limits<float>::max)());
		glm::vec3 hi(-(std::numeric_limits<float>::max)());
		for (int i = 0; i < 8; i++) {
			glm::vec4 corner((i & 1) ? 0.5f : -0.5f,
				(i & 2) ? 0.5f : -0.5f,
				(i & 4) ? 0.5f : -0.5f,
				1.0f);
			glm::vec3 p = glm::vec3(transform * corner);
			lo = (glm::min)(lo, p);
			hi = (glm::max)(hi, p);
		}
		return AABB(lo, hi);
	}

	// Bounds of an arbitrary AABB after `transform`.
	inline AABB transformAABB(const AABB& box, const glm::mat4& transform) {
		glm::vec3 lo((std::numeric_limits<float>::max)());
		glm::vec3 hi(-(std::numeric_limits<float>::max)());
		for (int i = 0; i < 8; i++) {
			glm::vec4 corner((i & 1) ? box.max.x : box.min.x,
				(i & 2) ? box.max.y : box.min.y,
				(i & 4) ? box.max.z : box.min.z,
				1.0f);
			glm::vec3 p = glm::vec3(transform * corner);
			lo = (glm::min)(lo, p);
			hi = (glm::max)(hi, p);
		}
		return AABB(lo, hi);
	}

	// Smallest horizontal (XZ) translation that moves `mover` out of `solid`.
	// Returns a zero vector when they do not overlap.
	inline glm::vec3 penetrationXZ(const AABB& mover, const AABB& solid) {
		if (!mover.intersects(solid))
			return glm::vec3(0.0f);

		float pushLeft = solid.min.x - mover.max.x;		// negative
		float pushRight = solid.max.x - mover.min.x;	// positive
		float pushBack = solid.min.z - mover.max.z;		// negative
		float pushFwd = solid.max.z - mover.min.z;		// positive

		float px = (std::abs(pushLeft) < std::abs(pushRight)) ? pushLeft : pushRight;
		float pz = (std::abs(pushBack) < std::abs(pushFwd)) ? pushBack : pushFwd;

		if (std::abs(px) < std::abs(pz))
			return glm::vec3(px, 0.0f, 0.0f);
		return glm::vec3(0.0f, 0.0f, pz);
	}
}

#endif // !COLLISION_HPP
