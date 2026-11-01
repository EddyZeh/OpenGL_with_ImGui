#include "LockBox.h"

#include "../Game/GameContext.h"

LockBox::LockBox(const Collision::AABB& body, Door& lid)
	: Interactable("lockbox"), body(body), lidPanel(lid) {}

bool LockBox::canTarget(const GameContext& ctx) const {
	// once open, the card inside is the thing to look at
	return !ctx.state.boxOpened;
}

std::string LockBox::prompt(const GameContext& ctx) const {
	if (ctx.state.hasKey)
		return "Press E to unlock the box with the key";
	return "Press E to inspect lockbox";
}

void LockBox::interact(GameContext& ctx) {
	if (ctx.puzzles.canOpenLockBox(ctx.state)) {
		ctx.state.boxOpened = true;
		ctx.ui.message("The key turns. The lid springs open.");
		return;
	}
	ctx.ui.message("A sturdy steel lockbox. The keyhole needs a small key.");
}

Collision::AABB LockBox::bounds(const GameContext& ctx) const {
	(void)ctx;
	return body.expanded(0.03f);
}

void LockBox::update(float dt, GameContext& ctx) {
	(void)dt;
	if (ctx.state.boxOpened && lidPanel.isLocked()) {
		lidPanel.unlock();
		lidPanel.open();
	}
}
