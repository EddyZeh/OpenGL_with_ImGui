#include "Door.h"

#include <glm/gtc/matrix_transform.hpp>

#include <cmath>

#include "../Game/Level.h"

Door::Door(std::string group, const glm::vec3& hinge, const glm::vec3& axis,
	float openAngleDegrees, float speedDegreesPerSecond)
	: groupName(std::move(group)), hinge(hinge), axis(glm::normalize(axis)),
	openAngle(openAngleDegrees), speed(speedDegreesPerSecond) {}

void Door::setPanel(Level& level, const glm::mat4& localUnitCube, const Surface& surface,
	bool solid, bool blocksView) {
	panelLocal = localUnitCube;
	panelProp = level.addProp(groupName, Shape::Cube, hingeMatrix() * panelLocal, surface);
	parts.push_back({ panelProp, panelLocal });

	if (solid)
		colliderIndex = level.addCollider(panelBounds(), groupName, blocksView);
}

int Door::addPart(Level& level, Shape shape, const glm::mat4& local, const Surface& surface, bool castsShadow) {
	int idx = level.addProp(groupName, shape, hingeMatrix() * local, surface, castsShadow);
	parts.push_back({ idx, local });
	return idx;
}

void Door::unlock() {
	if (currentState == State::Locked)
		currentState = State::Closed;
}

bool Door::open() {
	if (currentState == State::Closed) {
		currentState = State::Opening;
		return true;
	}
	return false;
}

float Door::openFraction() const {
	if (openAngle == 0.0f)
		return 0.0f;
	return angle / openAngle;
}

void Door::update(float dt, Level& level) {
	if (currentState == State::Opening) {
		float step = speed * dt;
		float remaining = openAngle - angle;

		if (std::abs(remaining) <= step) {
			angle = openAngle;
			currentState = State::Open;
		}
		else {
			angle += (remaining > 0.0f ? step : -step);
		}
		sync(level);
	}
}

glm::mat4 Door::hingeMatrix() const {
	glm::mat4 m = glm::translate(glm::mat4(1.0f), hinge);
	return glm::rotate(m, glm::radians(angle), axis);
}

Collision::AABB Door::panelBounds() const {
	return Collision::transformUnitCube(hingeMatrix() * panelLocal);
}

void Door::sync(Level& level) {
	glm::mat4 h = hingeMatrix();
	for (const auto& p : parts)
		level.props[p.prop].transform = h * p.local;

	if (colliderIndex >= 0)
		level.colliders[colliderIndex].box = panelBounds();

	level.setAnchor(groupName, h);
}
