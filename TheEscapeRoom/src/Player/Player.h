#ifndef ESCAPE_PLAYER_H
#define ESCAPE_PLAYER_H

#include <glm/glm.hpp>

#include <vector>

#include "graphics/Camera.h"		// engine FPS camera (orientation only)
#include "physics/collision.hpp"	// engine

#include "../Game/Level.h"

// Movement intent for one frame, already decoupled from GLFW / gamepad.
struct PlayerInput {
	float forward = 0.0f;	// -1 .. 1
	float right = 0.0f;		// -1 .. 1
	bool sprint = false;
	double lookDX = 0.0;	// mouse delta in pixels
	double lookDY = 0.0;
};

/*
	First-person player that obeys the world.

	The engine Camera provides yaw / pitch and the view vectors; the player owns its
	position and resolves movement against the level's AABB colliders using a box-shaped
	collision volume (not just the camera point), one axis at a time, in small sub-steps.
*/
class Player {
public:
	Player();

	void spawn(const glm::vec3& feet, float yawDegrees, float pitchDegrees = 0.0f);
	void update(float dt, const PlayerInput& input, const std::vector<Collider>& colliders);

	// separate step so it can also be called while the player stands still (doors swinging into them)
	void resolvePenetration(const std::vector<Collider>& colliders);

	Collision::AABB bounds() const { return boundsAt(feet); }
	Collision::AABB boundsAt(const glm::vec3& feetPosition) const;

	glm::vec3 eyePosition() const;
	glm::vec3 forward() const { return cam.cameraFront; }
	Collision::Ray viewRay() const;
	glm::mat4 viewMatrix() const { return cam.getViewMatrix(); }
	float fov() { return cam.getZoom(); }
	Camera& camera() { return cam; }

	glm::vec3 feet = glm::vec3(0.0f);

	// tuning
	float radius = 0.25f;			// half width of the collision box
	float height = 1.75f;
	float eyeHeight = 1.62f;
	float walkSpeed = 2.4f;
	float sprintSpeed = 4.0f;
	float lookSensitivity = 0.1f;	// degrees per pixel
	bool headBob = true;

private:
	Camera cam;
	float bobPhase = 0.0f;
	float bobAmount = 0.0f;

	void moveAxis(int axis, float amount, const std::vector<Collider>& colliders);
	void syncCamera();
};

#endif // !ESCAPE_PLAYER_H
