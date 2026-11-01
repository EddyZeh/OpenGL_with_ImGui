#ifndef ESCAPE_KEY_H
#define ESCAPE_KEY_H

#include <glm/glm.hpp>

#include <vector>

#include "../Interaction/Interactable.h"

class Door;

/*
	The key inside the cabinet. Collecting it updates the inventory (GameState::hasKey)
	and removes it from the world in the same step, so the two can never disagree.
*/
class Key : public Interactable {
public:
	Key(const glm::vec3& position, const Door& cabinetDoor);

	// key pieces in key space (origin = key centre); built by Level
	void addPiece(int propIndex, const glm::mat4& local);

	bool canTarget(const GameContext& ctx) const override;
	std::string prompt(const GameContext& ctx) const override;
	void interact(GameContext& ctx) override;
	Collision::AABB bounds(const GameContext& ctx) const override;
	void update(float dt, GameContext& ctx) override;

private:
	struct Piece {
		int prop;
		glm::mat4 local;
	};

	glm::vec3 position;
	const Door& cabinetDoor;
	std::vector<Piece> pieces;
	float spin = 0.0f;
};

#endif // !ESCAPE_KEY_H
