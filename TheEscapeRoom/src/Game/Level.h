#ifndef ESCAPE_LEVEL_H
#define ESCAPE_LEVEL_H

#include <glm/glm.hpp>

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "physics/collision.hpp"	// engine

#include "../Rendering/Prop.h"
#include "../Interaction/Interactable.h"
#include "../Objects/Door.h"

class PuzzleManager;

/*
	The level: what is in the room and where.

	It is pure data + game objects (no OpenGL calls), built in C++ from primitives.
	The renderer reads `props`, `lights` and `modelSlots`; the player collides against
	`colliders`; the interaction system ray casts against `interactables`.
*/

struct Collider {
	Collision::AABB box;
	std::string tag;
	bool enabled = true;
	bool blocksView = false;	// walls / doors also stop the interaction ray
};

enum class Flicker {
	None,
	Fluorescent,	// occasional dropouts
	Pulse			// slow emergency-light pulse
};

struct LevelLight {
	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 ambient = glm::vec3(0.0f);
	glm::vec3 diffuse = glm::vec3(1.0f);
	glm::vec3 specular = glm::vec3(1.0f);
	float k0 = 1.0f, k1 = 0.09f, k2 = 0.032f;
	bool castsShadow = false;
	Flicker flicker = Flicker::None;
	int fixtureProp = -1;		// emissive prop dimmed together with the light
};

// A placeholder group that has been swapped for an imported model.
struct ModelSlot {
	std::string group;
	std::string path;			// relative to assets/models
	glm::mat4 local = glm::mat4(1.0f);	// relative to the group's anchor
	bool noTex = false;			// true: use the model's material colours instead of textures
};

class Level {
public:
	Level() = default;
	Level(const Level&) = delete;
	Level& operator=(const Level&) = delete;

	// Clears everything and builds the escape room for the given puzzle.
	void build(const PuzzleManager& puzzles);
	void clear();

	// Advance doors / animations
	void update(float dt);

	// ---- construction helpers -------------------------------------------------------
	int addProp(const std::string& group, Shape shape, const glm::mat4& transform,
		const Surface& surface, bool castsShadow = true);
	int addCollider(const Collision::AABB& box, const std::string& tag, bool blocksView = false);

	// a visible box that is also solid
	int addSolidBox(const std::string& group, const glm::vec3& lo, const glm::vec3& hi,
		const Surface& surface, bool blocksView = false);

	Door& addDoor(std::unique_ptr<Door> door);

	template <class T, class... Args>
	T& addInteractable(Args&&... args) {
		interactables.push_back(std::unique_ptr<Interactable>(new T(std::forward<Args>(args)...)));
		return static_cast<T&>(*interactables.back());
	}

	// ---- placeholder -> model replacement --------------------------------------------
	// Every placeholder group has an anchor (its placement in the room). An imported model is
	// drawn at anchor * local, and the group's primitives are hidden.
	void setAnchor(const std::string& group, const glm::mat4& anchor);
	glm::mat4 anchor(const std::string& group) const;
	void replaceWithModel(const std::string& group, const std::string& modelPath,
		const glm::mat4& local = glm::mat4(1.0f), bool noTex = false);
	bool isReplaced(const std::string& group) const;

	// ---- data ----------------------------------------------------------------------
	std::vector<Prop> props;
	std::vector<Collider> colliders;
	std::vector<LevelLight> lights;
	std::vector<ModelSlot> modelSlots;
	std::vector<std::unique_ptr<Interactable>> interactables;
	std::vector<std::unique_ptr<Door>> doors;

	glm::vec3 playerSpawn = glm::vec3(0.0f);
	float spawnYaw = -90.0f;		// degrees, -90 looks down -Z (north)

	Collision::AABB exitZone;		// walking in here after the door opens wins the game
	Door* exitDoor = nullptr;

	// room extents (inside faces of the walls)
	static constexpr float ROOM_MIN_X = -6.0f, ROOM_MAX_X = 6.0f;
	static constexpr float ROOM_MIN_Z = -5.0f, ROOM_MAX_Z = 5.0f;
	static constexpr float ROOM_HEIGHT = 3.0f;

private:
	std::map<std::string, glm::mat4> anchors;

	void buildShell();
	void buildDesk();
	void buildCabinet();
	void buildTable();
	void buildClueBoard(const PuzzleManager& puzzles);
	void buildExit();
	void buildDecor();
	void buildLights();
	void applyModelReplacements();
};

#endif // !ESCAPE_LEVEL_H
