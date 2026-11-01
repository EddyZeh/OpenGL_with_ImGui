#include "Note.h"

#include "../Game/GameContext.h"

Readable::Readable(const std::string& group, const Collision::AABB& box, std::string promptText,
	std::string title, std::string body, Callback onRead, Condition available)
	: Interactable(group), box(box.withMinThickness(0.12f)), promptText(std::move(promptText)),
	title(std::move(title)), body(std::move(body)), onRead(std::move(onRead)), available(std::move(available)) {}

bool Readable::canTarget(const GameContext& ctx) const {
	return available ? available(ctx) : true;
}

std::string Readable::prompt(const GameContext& ctx) const {
	(void)ctx;
	return "Press E to " + promptText;
}

void Readable::interact(GameContext& ctx) {
	if (onRead)
		onRead(ctx);
	ctx.ui.read(title, body);
}

Collision::AABB Readable::bounds(const GameContext& ctx) const {
	(void)ctx;
	return box;
}
