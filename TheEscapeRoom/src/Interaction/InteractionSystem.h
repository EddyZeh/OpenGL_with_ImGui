#ifndef ESCAPE_INTERACTIONSYSTEM_H
#define ESCAPE_INTERACTIONSYSTEM_H

#include <string>

#include "physics/collision.hpp"	// engine

#include "Interactable.h"

class Level;
struct GameContext;

/*
	Finds what the player is looking at: a ray from the centre of the view is tested against
	every targetable object's AABB, the closest hit within range wins, and walls / closed
	doors in front of it block the ray.
*/
class InteractionSystem {
public:
	float maxDistance = 2.3f;

	// Recompute the current target. Call once per frame during gameplay.
	void update(const Collision::Ray& ray, const GameContext& ctx);

	// Player pressed E: forwards the request to the target (which decides the outcome).
	bool interact(GameContext& ctx);

	void clearTarget() { current = nullptr; distance = 0.0f; }

	Interactable* target() const { return current; }
	float targetDistance() const { return distance; }

private:
	Interactable* current = nullptr;
	float distance = 0.0f;
};

#endif // !ESCAPE_INTERACTIONSYSTEM_H
