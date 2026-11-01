#include "Player.h"

#include <cmath>

namespace {
	const float SKIN = 0.001f;		// keep a hair's gap from walls so we never start inside one
	const float MAX_STEP = 0.05f;	// largest movement per collision sub-step (no tunnelling)
}

Player::Player() : cam(glm::vec3(0.0f)) {}

void Player::spawn(const glm::vec3& feetPosition, float yawDegrees, float pitchDegrees) {
	feet = feetPosition;
	cam.yaw = yawDegrees;
	cam.pitch = pitchDegrees;
	cam.zoom = 45.0f;
	cam.updateCameraDirection(0.0, 0.0);	// recompute the view vectors
	bobPhase = 0.0f;
	bobAmount = 0.0f;
	syncCamera();
}

Collision::AABB Player::boundsAt(const glm::vec3& f) const {
	return Collision::AABB(glm::vec3(f.x - radius, f.y, f.z - radius),
		glm::vec3(f.x + radius, f.y + height, f.z + radius));
}

glm::vec3 Player::eyePosition() const {
	float bob = headBob ? std::sin(bobPhase) * 0.035f * bobAmount : 0.0f;
	return feet + glm::vec3(0.0f, eyeHeight + bob, 0.0f);
}

Collision::Ray Player::viewRay() const {
	return { eyePosition(), glm::normalize(cam.cameraFront) };
}

void Player::update(float dt, const PlayerInput& input, const std::vector<Collider>& colliders) {
	// ---- look ------------------------------------------------------------------------
	if (input.lookDX != 0.0 || input.lookDY != 0.0)
		cam.updateCameraDirection(input.lookDX * lookSensitivity, input.lookDY * lookSensitivity);

	// ---- walk on the XZ plane (looking up must not slow you down) -----------------------
	glm::vec3 flatFront(cam.cameraFront.x, 0.0f, cam.cameraFront.z);
	if (glm::dot(flatFront, flatFront) < 1e-6f)
		flatFront = glm::vec3(std::cos(glm::radians(cam.yaw)), 0.0f, std::sin(glm::radians(cam.yaw)));
	flatFront = glm::normalize(flatFront);
	glm::vec3 flatRight = glm::normalize(glm::cross(flatFront, glm::vec3(0.0f, 1.0f, 0.0f)));

	glm::vec3 wish = flatFront * input.forward + flatRight * input.right;
	float len = glm::length(wish);
	if (len > 1.0f)
		wish /= len;

	float speed = input.sprint ? sprintSpeed : walkSpeed;
	glm::vec3 delta = wish * speed * dt;

	// get out of anything that moved into us (a swinging door) before moving
	resolvePenetration(colliders);

	float dist = glm::length(delta);
	int steps = (int)std::ceil(dist / MAX_STEP);
	if (steps < 1)
		steps = 1;
	glm::vec3 step = delta / (float)steps;

	for (int i = 0; i < steps; i++) {
		moveAxis(0, step.x, colliders);
		moveAxis(2, step.z, colliders);
	}

	// ---- head bob ---------------------------------------------------------------------
	float moving = (len > 0.01f) ? 1.0f : 0.0f;
	bobAmount += (moving - bobAmount) * (glm::min)(1.0f, dt * 8.0f);
	bobPhase += dt * (input.sprint ? 13.0f : 9.0f) * moving;

	syncCamera();
}

void Player::moveAxis(int axis, float amount, const std::vector<Collider>& colliders) {
	if (amount == 0.0f)
		return;

	feet[axis] += amount;
	Collision::AABB box = bounds();

	for (const auto& c : colliders) {
		if (!c.enabled || !box.intersects(c.box))
			continue;

		// push back to the face we ran into
		if (amount > 0.0f)
			feet[axis] = c.box.min[axis] - radius - SKIN;
		else
			feet[axis] = c.box.max[axis] + radius + SKIN;

		box = bounds();
	}
}

void Player::resolvePenetration(const std::vector<Collider>& colliders) {
	for (int iteration = 0; iteration < 4; iteration++) {
		bool moved = false;
		for (const auto& c : colliders) {
			if (!c.enabled)
				continue;
			glm::vec3 push = Collision::penetrationXZ(bounds(), c.box);
			if (push.x != 0.0f || push.z != 0.0f) {
				// add the skin in the direction of the push
				push.x += (push.x > 0.0f) ? SKIN : (push.x < 0.0f ? -SKIN : 0.0f);
				push.z += (push.z > 0.0f) ? SKIN : (push.z < 0.0f ? -SKIN : 0.0f);
				feet += push;
				moved = true;
			}
		}
		if (!moved)
			break;
	}
	syncCamera();
}

void Player::syncCamera() {
	cam.cameraPos = eyePosition();
}
