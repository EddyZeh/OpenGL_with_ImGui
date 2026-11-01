#ifndef ESCAPE_CLUEBOARD_H
#define ESCAPE_CLUEBOARD_H

#include "../Interaction/Interactable.h"

/*
	Wall-mounted board (area D) with four rows of coloured indicator lights.
	The number of lights in each row, top to bottom, is the cabinet combination.
	The clue is the geometry itself, so there is no tiny text to squint at.
*/
class ClueBoard : public Interactable {
public:
	explicit ClueBoard(const Collision::AABB& box);

	std::string prompt(const GameContext& ctx) const override;
	void interact(GameContext& ctx) override;
	Collision::AABB bounds(const GameContext& ctx) const override;

private:
	Collision::AABB box;
};

#endif // !ESCAPE_CLUEBOARD_H
