#ifndef ESCAPE_CABINET_H
#define ESCAPE_CABINET_H

#include "../Interaction/Interactable.h"
#include "Door.h"

/*
	Locked metal cabinet (area B). Using it opens the keypad; once the PuzzleManager
	accepts the code the door swings open and the key inside becomes reachable.
*/
class Cabinet : public Interactable {
public:
	Cabinet(const Collision::AABB& body, Door& door, int statusLightProp);

	bool canTarget(const GameContext& ctx) const override;
	std::string prompt(const GameContext& ctx) const override;
	void interact(GameContext& ctx) override;
	Collision::AABB bounds(const GameContext& ctx) const override;
	void update(float dt, GameContext& ctx) override;

	const Door& door() const { return cabinetDoor; }

private:
	Collision::AABB body;
	Door& cabinetDoor;
	int statusLight;
	float blinkTimer = 0.0f;
};

#endif // !ESCAPE_CABINET_H
