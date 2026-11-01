#include "InteractionSystem.h"

#include "../Game/GameContext.h"
#include "../Game/Level.h"

void InteractionSystem::update(const Collision::Ray& ray, const GameContext& ctx) {
	current = nullptr;
	distance = maxDistance;

	// closest targetable object within range
	for (auto& obj : ctx.level.interactables) {
		if (!obj->canTarget(ctx))
			continue;

		float t;
		if (Collision::rayVsAABB(ray, obj->bounds(ctx), t) && t <= distance) {
			current = obj.get();
			distance = t;
		}
	}

	if (!current)
		return;

	// anything solid and opaque in front of it (e.g. aiming at the keypad through the door frame)
	for (const auto& c : ctx.level.colliders) {
		if (!c.enabled || !c.blocksView)
			continue;

		// the target's own collider (the exit door) never hides it
		if (c.tag == current->group())
			continue;

		float t;
		if (Collision::rayVsAABB(ray, c.box, t) && t + 0.01f < distance && t > 0.0f) {
			current = nullptr;
			distance = 0.0f;
			return;
		}
	}
}

bool InteractionSystem::interact(GameContext& ctx) {
	if (!current || !current->canTarget(ctx))
		return false;

	current->interact(ctx);
	return true;
}
