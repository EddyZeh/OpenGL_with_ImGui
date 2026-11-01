#include "ClueBoard.h"

#include "../Game/GameContext.h"

ClueBoard::ClueBoard(const Collision::AABB& box)
	: Interactable("clueboard"), box(box.withMinThickness(0.15f)) {}

std::string ClueBoard::prompt(const GameContext& ctx) const {
	(void)ctx;
	return "Press E to inspect the indicator board";
}

void ClueBoard::interact(GameContext& ctx) {
	ctx.state.hasInspectedBoard = true;

	if (ctx.state.hasReadNote)
		ctx.ui.message("Red, green, blue, yellow... count the lights in each row, top to bottom.");
	else
		ctx.ui.message("Rows of coloured indicator lights. Some are lit, some aren't wired at all.");
}

Collision::AABB ClueBoard::bounds(const GameContext& ctx) const {
	(void)ctx;
	return box;
}
