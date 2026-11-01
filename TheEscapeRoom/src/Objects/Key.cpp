#include "Key.h"

#include <glm/gtc/matrix_transform.hpp>

#include <cmath>

#include "Door.h"
#include "../Game/GameContext.h"
#include "../Game/Level.h"

Key::Key(const glm::vec3& position, const Door& cabinetDoor)
	: Interactable("key"), position(position), cabinetDoor(cabinetDoor) {}

void Key::addPiece(int propIndex, const glm::mat4& local) {
	pieces.push_back({ propIndex, local });
}

bool Key::canTarget(const GameContext& ctx) const {
	// only once the cabinet has been unlocked and its door is mostly open
	return ctx.puzzles.isKeyAvailable(ctx.state) && cabinetDoor.openFraction() > 0.6f;
}

std::string Key::prompt(const GameContext& ctx) const {
	(void)ctx;
	return "Press E to pick up key";
}

void Key::interact(GameContext& ctx) {
	if (ctx.state.hasKey)
		return;		// cannot be collected twice

	ctx.state.hasKey = true;
	for (const auto& p : pieces)
		ctx.level.props[p.prop].visible = false;

	ctx.ui.message("Picked up: small brass key");
}

Collision::AABB Key::bounds(const GameContext& ctx) const {
	(void)ctx;
	return Collision::AABB::fromCenterSize(position, glm::vec3(0.35f, 0.3f, 0.35f));
}

void Key::update(float dt, GameContext& ctx) {
	if (ctx.state.hasKey)
		return;

	// slow spin + bob so it catches the eye when the cabinet opens
	spin += dt;
	glm::mat4 base = glm::translate(glm::mat4(1.0f), position + glm::vec3(0.0f, 0.02f * std::sin(spin * 2.0f), 0.0f));
	base = glm::rotate(base, spin * 0.9f, glm::vec3(0.0f, 1.0f, 0.0f));

	for (const auto& p : pieces)
		ctx.level.props[p.prop].transform = base * p.local;

	ctx.level.setAnchor("key", base);
}
