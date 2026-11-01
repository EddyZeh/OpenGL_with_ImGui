#ifndef ESCAPE_INTERACTABLE_H
#define ESCAPE_INTERACTABLE_H

#include <string>
#include <utility>

#include "physics/collision.hpp"	// engine

struct GameContext;

/*
	Common interface for anything the player can aim at and press E on.

	Pressing E only *requests* an interaction: interact() receives the game context and
	decides, using the current game state and the PuzzleManager rules, what actually happens.
*/
class Interactable {
public:
	explicit Interactable(std::string group) : groupName(std::move(group)) {}
	virtual ~Interactable() = default;

	// Can the player currently aim at this? (e.g. the key is hidden until the cabinet opens)
	virtual bool canTarget(const GameContext& ctx) const { (void)ctx; return true; }

	// The text shown in the HUD, e.g. "Press E to read note"
	virtual std::string prompt(const GameContext& ctx) const = 0;

	// What happens when the player presses E while aiming at this object
	virtual void interact(GameContext& ctx) = 0;

	// Volume used by the interaction ray cast (not necessarily the collision volume)
	virtual Collision::AABB bounds(const GameContext& ctx) const = 0;

	// Per-frame update for animations / reacting to state changes
	virtual void update(float dt, GameContext& ctx) { (void)dt; (void)ctx; }

	// Props in this group are highlighted while targeted
	const std::string& group() const { return groupName; }

private:
	std::string groupName;
};

#endif // !ESCAPE_INTERACTABLE_H
