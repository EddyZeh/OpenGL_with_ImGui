#ifndef ESCAPE_DOOR_H
#define ESCAPE_DOOR_H

#include <glm/glm.hpp>

#include <string>
#include <utility>
#include <vector>

#include "physics/collision.hpp"	// engine

#include "../Rendering/Prop.h"

class Level;

/*
	A hinged panel with a lock: the exit door, the cabinet door and the lockbox lid.

	Locked and Open are different things: unlocking only allows the door to be opened.
	The visual transform and the collision box are both derived from the same hinge angle,
	so an open door is never left with an invisible wall behind it.

	Parts are modelled in "hinge space" (origin on the hinge line, closed position), so
	rotating about the hinge is a translate-rotate sequence rather than a rotation about
	the panel's centre.
*/
class Door {
public:
	enum class State {
		Locked,
		Closed,		// unlocked but shut
		Opening,
		Open
	};

	Door(std::string group, const glm::vec3& hinge, const glm::vec3& axis,
		float openAngleDegrees, float speedDegreesPerSecond = 110.0f);

	// The main slab. Its rotated bounds become the door's collider (when `solid`).
	void setPanel(Level& level, const glm::mat4& localUnitCube, const Surface& surface,
		bool solid, bool blocksView);

	// Extra pieces that swing with the door (handle, keypad light, ...). Returns the prop index.
	int addPart(Level& level, Shape shape, const glm::mat4& local, const Surface& surface,
		bool castsShadow = true);

	void unlock();
	bool open();				// starts opening; false if still locked
	void update(float dt, Level& level);

	State state() const { return currentState; }
	bool isLocked() const { return currentState == State::Locked; }
	bool isOpen() const { return currentState == State::Open; }
	bool isOpeningOrOpen() const { return currentState == State::Opening || currentState == State::Open; }
	float openFraction() const;

	glm::mat4 hingeMatrix() const;
	Collision::AABB panelBounds() const;

	const std::string& group() const { return groupName; }

	// snaps the transforms to the current angle (call after construction)
	void sync(Level& level);

private:
	struct Part {
		int prop;
		glm::mat4 local;
	};

	std::string groupName;
	glm::vec3 hinge;
	glm::vec3 axis;
	float openAngle;
	float speed;
	float angle = 0.0f;

	State currentState = State::Locked;

	glm::mat4 panelLocal = glm::mat4(1.0f);
	int panelProp = -1;
	int colliderIndex = -1;
	std::vector<Part> parts;
};

#endif // !ESCAPE_DOOR_H
