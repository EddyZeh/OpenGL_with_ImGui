#ifndef ESCAPE_EXITDOOR_H
#define ESCAPE_EXITDOOR_H

#include "../Interaction/Interactable.h"
#include "Door.h"

/*
	The facility exit (area E). It only opens after the exit keypad accepts the code;
	the PuzzleManager owns that rule.
*/
class ExitDoor : public Interactable {
public:
	explicit ExitDoor(Door& door);

	bool canTarget(const GameContext& ctx) const override;
	std::string prompt(const GameContext& ctx) const override;
	void interact(GameContext& ctx) override;
	Collision::AABB bounds(const GameContext& ctx) const override;
	void update(float dt, GameContext& ctx) override;

private:
	Door& door;
};

#endif // !ESCAPE_EXITDOOR_H
