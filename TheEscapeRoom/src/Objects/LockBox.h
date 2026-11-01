#ifndef ESCAPE_LOCKBOX_H
#define ESCAPE_LOCKBOX_H

#include "../Interaction/Interactable.h"
#include "Door.h"

/*
	Small locked box on the table (area C). The key from the cabinet opens it,
	revealing the card with the exit code.
*/
class LockBox : public Interactable {
public:
	LockBox(const Collision::AABB& body, Door& lid);

	bool canTarget(const GameContext& ctx) const override;
	std::string prompt(const GameContext& ctx) const override;
	void interact(GameContext& ctx) override;
	Collision::AABB bounds(const GameContext& ctx) const override;
	void update(float dt, GameContext& ctx) override;

	const Door& lid() const { return lidPanel; }

private:
	Collision::AABB body;
	Door& lidPanel;
};

#endif // !ESCAPE_LOCKBOX_H
