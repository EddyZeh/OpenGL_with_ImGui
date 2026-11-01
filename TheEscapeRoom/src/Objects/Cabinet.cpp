#include "Cabinet.h"

#include <cmath>

#include "../Game/GameContext.h"
#include "../Game/Level.h"

Cabinet::Cabinet(const Collision::AABB& body, Door& door, int statusLightProp)
	: Interactable("cabinet"), body(body), cabinetDoor(door), statusLight(statusLightProp) {}

bool Cabinet::canTarget(const GameContext& ctx) const {
	// while the key is inside and the door is open, let the ray reach the key instead
	if (cabinetDoor.isOpeningOrOpen() && !ctx.state.hasKey)
		return false;
	return true;
}

std::string Cabinet::prompt(const GameContext& ctx) const {
	if (!ctx.state.cabinetUnlocked)
		return "Press E to use the cabinet keypad";
	return "Press E to inspect cabinet";
}

void Cabinet::interact(GameContext& ctx) {
	if (!ctx.state.cabinetUnlocked) {
		ctx.ui.keypad(KeypadTarget::Cabinet);
		return;
	}
	if (ctx.state.hasKey)
		ctx.ui.message("The cabinet is empty now.");
}

Collision::AABB Cabinet::bounds(const GameContext& ctx) const {
	(void)ctx;
	return body;
}

void Cabinet::update(float dt, GameContext& ctx) {
	// the door reacts to the game state, it never decides to open by itself
	if (ctx.state.cabinetUnlocked && cabinetDoor.isLocked()) {
		cabinetDoor.unlock();
		cabinetDoor.open();
		ctx.ui.message("*click* The cabinet door swings open.");
	}

	if (statusLight >= 0) {
		Surface& s = ctx.level.props[statusLight].surface;
		if (ctx.state.cabinetUnlocked) {
			s = Surface::glow(glm::vec3(0.1f, 1.0f, 0.3f), 1.2f);
		}
		else {
			blinkTimer += dt;
			float on = (std::fmod(blinkTimer, 1.2f) < 0.15f) ? 1.4f : 0.45f;
			s = Surface::glow(glm::vec3(1.0f, 0.1f, 0.05f), on);
		}
	}
}
