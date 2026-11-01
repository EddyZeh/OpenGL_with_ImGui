#ifndef ESCAPE_NOTE_H
#define ESCAPE_NOTE_H

#include <functional>
#include <string>

#include "../Interaction/Interactable.h"

/*
	Something with text on it: the note on the desk, the card inside the lockbox.
	Reading opens the note window; an optional callback records that it was read.
*/
class Readable : public Interactable {
public:
	using Condition = std::function<bool(const GameContext&)>;
	using Callback = std::function<void(GameContext&)>;

	Readable(const std::string& group, const Collision::AABB& box, std::string promptText,
		std::string title, std::string body, Callback onRead = {}, Condition available = {});

	bool canTarget(const GameContext& ctx) const override;
	std::string prompt(const GameContext& ctx) const override;
	void interact(GameContext& ctx) override;
	Collision::AABB bounds(const GameContext& ctx) const override;

private:
	Collision::AABB box;
	std::string promptText;
	std::string title;
	std::string body;
	Callback onRead;
	Condition available;
};

#endif // !ESCAPE_NOTE_H
