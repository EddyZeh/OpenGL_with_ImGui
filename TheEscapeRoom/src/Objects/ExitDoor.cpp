#include "ExitDoor.h"

#include "../Game/GameContext.h"

ExitDoor::ExitDoor(Door& door)
	: Interactable(door.group()), door(door) {}

bool ExitDoor::canTarget(const GameContext& ctx) const {
	(void)ctx;
	return !door.isOpeningOrOpen();
}

std::string ExitDoor::prompt(const GameContext& ctx) const {
	if (ctx.puzzles.canOpenExitDoor(ctx.state))
		return "Press E to open door";
	return "Press E to try the door";
}

void ExitDoor::interact(GameContext& ctx) {
	if (!ctx.puzzles.canOpenExitDoor(ctx.state)) {
		ctx.ui.message("Locked tight. The keypad beside it blinks red.");
		return;
	}

	door.unlock();
	if (door.open())
		ctx.ui.message("The heavy door grinds open. Fresh air...");
}

Collision::AABB ExitDoor::bounds(const GameContext& ctx) const {
	(void)ctx;
	return door.panelBounds().expanded(0.02f);
}

void ExitDoor::update(float dt, GameContext& ctx) {
	(void)dt;
	// unlocking (keypad) and opening (E on the door) are separate steps
	if (ctx.state.exitUnlocked && door.isLocked())
		door.unlock();
}
