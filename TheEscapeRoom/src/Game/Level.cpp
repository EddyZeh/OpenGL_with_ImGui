#include "Level.h"

#include <glm/gtc/matrix_transform.hpp>

#include "../Puzzles/PuzzleManager.h"
#include "GameContext.h"

#include "../Objects/Cabinet.h"
#include "../Objects/ClueBoard.h"
#include "../Objects/ExitDoor.h"
#include "../Objects/Key.h"
#include "../Objects/Keypad.h"
#include "../Objects/LockBox.h"
#include "../Objects/Note.h"

/*
	Room layout (top view, north = -Z, east = +X). One unit = one metre.

	                     NORTH WALL (z = -5)
	  +-----------------------------------------------------+
	  | [A] DESK + lamp                       [B] CABINET   |
	  |      chair                                          |
	  | crates                                              |
	  |                     [C] TABLE                       |
	  |                       lockbox            barrels    |
	  | [D] CLUE                                            |
	  |     BOARD                                           |
	  |              cot (spawn)            keypad  [E] EXIT|
	  +----------------------------------------+====+-------+
	                     SOUTH WALL (z = 5)     corridor -> escape
*/

using Collision::AABB;
using glm::vec3;

namespace {
	// ---- palette -------------------------------------------------------------------------
	const vec3 STEEL(0.30f, 0.34f, 0.38f);
	const vec3 DARK_STEEL(0.16f, 0.18f, 0.20f);
	const vec3 CHROME(0.70f, 0.72f, 0.75f);
	const vec3 RUST(0.42f, 0.22f, 0.12f);
	const vec3 PAPER(0.92f, 0.90f, 0.82f);
	const vec3 FABRIC(0.28f, 0.33f, 0.30f);
	const vec3 HAZARD(0.85f, 0.65f, 0.05f);

	Surface wallSurface() { return Surface::textured(TextureId::Brick, glm::vec2(0.6f), vec3(0.62f, 0.60f, 0.60f)); }
	Surface floorSurface() { return Surface::textured(TextureId::Concrete, glm::vec2(0.5f), vec3(0.36f, 0.37f, 0.40f)); }
	Surface ceilingSurface() { return Surface::colour(vec3(0.20f, 0.20f, 0.21f), 0.05f, 8.0f); }
	Surface woodSurface(float tint = 1.0f) { return Surface::textured(TextureId::Wood, glm::vec2(1.5f), vec3(tint)); }
	Surface metal(vec3 c, float spec = 0.6f) { return Surface::colour(c, spec, 64.0f); }

	// imported models are optional: flip this to try the model-replacement path
	constexpr bool USE_IMPORTED_MODELS = false;
}

// =========================================================================== Level basics

void Level::clear() {
	props.clear();
	colliders.clear();
	lights.clear();
	modelSlots.clear();
	interactables.clear();
	doors.clear();
	anchors.clear();
	exitDoor = nullptr;
}

int Level::addProp(const std::string& group, Shape shape, const glm::mat4& transform,
	const Surface& surface, bool castsShadow) {
	Prop p;
	p.group = group;
	p.shape = shape;
	p.transform = transform;
	p.surface = surface;
	p.castsShadow = castsShadow;
	props.push_back(p);
	return (int)props.size() - 1;
}

int Level::addCollider(const AABB& box, const std::string& tag, bool blocksView) {
	Collider c;
	c.box = box;
	c.tag = tag;
	c.blocksView = blocksView;
	colliders.push_back(c);
	return (int)colliders.size() - 1;
}

int Level::addSolidBox(const std::string& group, const vec3& lo, const vec3& hi,
	const Surface& surface, bool blocksView) {
	addCollider(AABB(lo, hi), group, blocksView);
	return addProp(group, Shape::Cube, Xform::span(lo, hi), surface);
}

Door& Level::addDoor(std::unique_ptr<Door> door) {
	doors.push_back(std::move(door));
	return *doors.back();
}

void Level::setAnchor(const std::string& group, const glm::mat4& a) {
	anchors[group] = a;
}

glm::mat4 Level::anchor(const std::string& group) const {
	auto it = anchors.find(group);
	return it != anchors.end() ? it->second : glm::mat4(1.0f);
}

void Level::replaceWithModel(const std::string& group, const std::string& modelPath,
	const glm::mat4& local, bool noTex) {
	for (auto& slot : modelSlots) {
		if (slot.group == group) {
			slot.path = modelPath;
			slot.local = local;
			slot.noTex = noTex;
			return;
		}
	}
	modelSlots.push_back({ group, modelPath, local, noTex });
}

bool Level::isReplaced(const std::string& group) const {
	for (const auto& slot : modelSlots)
		if (slot.group == group)
			return true;
	return false;
}

void Level::update(float dt) {
	for (auto& door : doors)
		door->update(dt, *this);
}

// =========================================================================== build

void Level::build(const PuzzleManager& puzzles) {
	clear();

	buildShell();
	buildDesk();
	buildCabinet();
	buildTable();
	buildClueBoard(puzzles);
	buildExit();
	buildDecor();
	buildLights();

	// the player wakes up next to the cot, facing into the room
	playerSpawn = vec3(-2.2f, 0.0f, 3.4f);
	spawnYaw = -70.0f;

	// ---- interactables that depend on several areas --------------------------------------
	// exit card inside the lockbox: its text holds the exit code
	const Door* lid = nullptr;
	for (auto& d : doors)
		if (d->group() == "lockbox")
			lid = d.get();

	std::string cardText =
		"MAINTENANCE OVERRIDE\n"
		"--------------------\n\n"
		"Exit door, level B3:\n\n"
		"        " + std::string(1, puzzles.exitCode()[0]) + "  " + puzzles.exitCode()[1] + "  " +
		puzzles.exitCode()[2] + "  " + puzzles.exitCode()[3] + "\n\n"
		"Destroy after reading.";

	addInteractable<Readable>("exit_card",
		AABB::fromCenterSize(vec3(0.0f, 0.82f, 0.30f), vec3(0.30f, 0.08f, 0.22f)),
		"read the card", "Override card", cardText,
		[](GameContext& ctx) { ctx.state.knowsExitCode = true; },
		[lid](const GameContext& ctx) { return ctx.state.boxOpened && lid && lid->openFraction() > 0.7f; });

	for (auto& d : doors)
		d->sync(*this);

	applyModelReplacements();
}

// --------------------------------------------------------------------------- shell: walls, floor, corridor

void Level::buildShell() {
	const float T = 0.3f;		// wall thickness
	const float H = ROOM_HEIGHT;

	// floor + ceiling (the player never moves vertically, so no colliders needed)
	addProp("room", Shape::Cube, Xform::span(vec3(ROOM_MIN_X - T, -0.1f, ROOM_MIN_Z - T), vec3(ROOM_MAX_X + T, 0.0f, ROOM_MAX_Z + T)), floorSurface());
	addProp("room", Shape::Cube, Xform::span(vec3(ROOM_MIN_X - T, H, ROOM_MIN_Z - T), vec3(ROOM_MAX_X + T, H + 0.1f, ROOM_MAX_Z + T)), ceilingSurface());

	// north / west / east walls
	addSolidBox("room", vec3(ROOM_MIN_X - T, 0.0f, ROOM_MIN_Z - T), vec3(ROOM_MAX_X + T, H, ROOM_MIN_Z), wallSurface(), true);
	addSolidBox("room", vec3(ROOM_MIN_X - T, 0.0f, ROOM_MIN_Z), vec3(ROOM_MIN_X, H, ROOM_MAX_Z), wallSurface(), true);
	addSolidBox("room", vec3(ROOM_MAX_X, 0.0f, ROOM_MIN_Z), vec3(ROOM_MAX_X + T, H, ROOM_MAX_Z), wallSurface(), true);

	// south wall with the exit doorway (x 3.4 .. 4.6, 2.2 m high)
	const float doorL = 3.4f, doorR = 4.6f, doorH = 2.2f;
	addSolidBox("room", vec3(ROOM_MIN_X - T, 0.0f, ROOM_MAX_Z), vec3(doorL, H, ROOM_MAX_Z + T), wallSurface(), true);
	addSolidBox("room", vec3(doorR, 0.0f, ROOM_MAX_Z), vec3(ROOM_MAX_X + T, H, ROOM_MAX_Z + T), wallSurface(), true);
	addSolidBox("room", vec3(doorL, doorH, ROOM_MAX_Z), vec3(doorR, H, ROOM_MAX_Z + T), wallSurface(), true);

	// door frame trim (visual only, the wall colliders already cover it)
	Surface frame = metal(DARK_STEEL, 0.4f);
	addProp("room", Shape::Cube, Xform::span(vec3(doorL - 0.1f, 0.0f, ROOM_MAX_Z - 0.03f), vec3(doorL, doorH + 0.1f, ROOM_MAX_Z + T + 0.03f)), frame);
	addProp("room", Shape::Cube, Xform::span(vec3(doorR, 0.0f, ROOM_MAX_Z - 0.03f), vec3(doorR + 0.1f, doorH + 0.1f, ROOM_MAX_Z + T + 0.03f)), frame);
	addProp("room", Shape::Cube, Xform::span(vec3(doorL - 0.1f, doorH, ROOM_MAX_Z - 0.03f), vec3(doorR + 0.1f, doorH + 0.1f, ROOM_MAX_Z + T + 0.03f)), frame);

	// skirting along the walls for a bit of structure
	Surface skirting = Surface::colour(vec3(0.12f, 0.12f, 0.13f), 0.2f, 16.0f);
	addProp("room", Shape::Cube, Xform::span(vec3(ROOM_MIN_X, 0.0f, ROOM_MIN_Z), vec3(ROOM_MAX_X, 0.12f, ROOM_MIN_Z + 0.02f)), skirting);
	addProp("room", Shape::Cube, Xform::span(vec3(ROOM_MIN_X, 0.0f, ROOM_MIN_Z), vec3(ROOM_MIN_X + 0.02f, 0.12f, ROOM_MAX_Z)), skirting);
	addProp("room", Shape::Cube, Xform::span(vec3(ROOM_MAX_X - 0.02f, 0.0f, ROOM_MIN_Z), vec3(ROOM_MAX_X, 0.12f, ROOM_MAX_Z)), skirting);

	// ---- exit corridor behind the door --------------------------------------------------
	const float cL = 3.2f, cR = 4.8f, cEnd = 8.5f, cH = 2.6f;
	const float z0 = ROOM_MAX_Z + T;
	addProp("corridor", Shape::Cube, Xform::span(vec3(cL - T, -0.1f, z0), vec3(cR + T, 0.0f, cEnd + T)), floorSurface());
	addProp("corridor", Shape::Cube, Xform::span(vec3(cL - T, cH, z0), vec3(cR + T, cH + 0.1f, cEnd + T)), ceilingSurface());
	addSolidBox("corridor", vec3(cL - T, 0.0f, z0), vec3(cL, cH, cEnd + T), wallSurface(), true);
	addSolidBox("corridor", vec3(cR, 0.0f, z0), vec3(cR + T, cH, cEnd + T), wallSurface(), true);
	addSolidBox("corridor", vec3(cL - T, 0.0f, cEnd), vec3(cR + T, cH, cEnd + T), wallSurface(), true);

	// glowing EXIT sign at the far end
	addProp("corridor", Shape::Cube, Xform::box(vec3(4.0f, 2.15f, cEnd - 0.03f), vec3(0.7f, 0.22f, 0.04f)),
		Surface::glow(vec3(0.2f, 1.0f, 0.4f), 1.4f), false);
	// guide strip on the corridor floor
	addProp("corridor", Shape::Cube, Xform::span(vec3(3.95f, 0.0f, z0), vec3(4.05f, 0.005f, cEnd - 0.2f)),
		Surface::glow(vec3(0.2f, 1.0f, 0.4f), 0.35f), false);

	exitZone = AABB(vec3(cL, 0.0f, 7.0f), vec3(cR, cH, cEnd));
	setAnchor("corridor", glm::mat4(1.0f));
	setAnchor("room", glm::mat4(1.0f));
}

// --------------------------------------------------------------------------- A: desk, chair, lamp, note

void Level::buildDesk() {
	const vec3 lo(-5.0f, 0.0f, -4.6f), hi(-3.4f, 0.79f, -3.8f);
	setAnchor("desk", glm::translate(glm::mat4(1.0f), vec3(-4.2f, 0.0f, -4.2f)));

	// PLACEHOLDER: desk = top + 4 legs + drawer unit
	addProp("desk", Shape::Cube, Xform::span(vec3(lo.x, 0.74f, lo.z), hi), woodSurface());
	const float legX[2] = { lo.x + 0.05f, hi.x - 0.05f };
	const float legZ[2] = { lo.z + 0.05f, hi.z - 0.05f };
	for (float x : legX)
		for (float z : legZ)
			addProp("desk", Shape::Cube, Xform::box(vec3(x, 0.37f, z), vec3(0.06f, 0.74f, 0.06f)), woodSurface(0.7f));
	addProp("desk", Shape::Cube, Xform::span(vec3(-3.95f, 0.30f, -4.55f), vec3(-3.47f, 0.74f, -3.86f)), woodSurface(0.8f));
	addProp("desk", Shape::Cube, Xform::box(vec3(-3.71f, 0.55f, -3.855f), vec3(0.14f, 0.025f, 0.02f)), metal(CHROME));
	addCollider(AABB(lo, hi), "desk");

	// PLACEHOLDER: chair
	setAnchor("chair", glm::translate(glm::mat4(1.0f), vec3(-4.2f, 0.0f, -3.35f)));
	Surface chair = metal(DARK_STEEL, 0.3f);
	addProp("chair", Shape::Cube, Xform::span(vec3(-4.45f, 0.42f, -3.6f), vec3(-3.95f, 0.47f, -3.1f)), Surface::colour(vec3(0.35f, 0.12f, 0.10f), 0.1f, 8.0f));
	addProp("chair", Shape::Cube, Xform::span(vec3(-4.45f, 0.47f, -3.13f), vec3(-3.95f, 0.98f, -3.08f)), Surface::colour(vec3(0.35f, 0.12f, 0.10f), 0.1f, 8.0f));
	const float cX[2] = { -4.42f, -3.98f };
	const float cZ[2] = { -3.57f, -3.13f };
	for (float x : cX)
		for (float z : cZ)
			addProp("chair", Shape::Cylinder, Xform::box(vec3(x, 0.21f, z), vec3(0.035f, 0.42f, 0.035f)), chair);
	addCollider(AABB(vec3(-4.45f, 0.0f, -3.6f), vec3(-3.95f, 0.98f, -3.08f)), "chair");

	// PLACEHOLDER: desk lamp (its bulb is a light source, see buildLights)
	setAnchor("desk_lamp", glm::translate(glm::mat4(1.0f), vec3(-4.75f, 0.79f, -4.4f)));
	Surface lampMetal = metal(vec3(0.10f, 0.25f, 0.18f), 0.5f);
	addProp("desk_lamp", Shape::Cylinder, Xform::box(vec3(-4.75f, 0.80f, -4.4f), vec3(0.18f, 0.025f, 0.18f)), lampMetal);
	addProp("desk_lamp", Shape::Cylinder, Xform::box(vec3(-4.75f, 0.99f, -4.4f), vec3(0.03f, 0.38f, 0.03f)), metal(CHROME));
	addProp("desk_lamp", Shape::Cylinder, Xform::cylinderAlong(vec3(-4.63f, 1.17f, -4.4f), 0.25f, 0.03f, 'x'), metal(CHROME));
	addProp("desk_lamp", Shape::Cylinder, Xform::box(vec3(-4.50f, 1.13f, -4.4f), vec3(0.17f, 0.10f, 0.17f)), lampMetal, false);

	// PLACEHOLDER: clutter
	addProp("desk_clutter", Shape::Cube, Xform::boxRotY(vec3(-3.65f, 0.82f, -4.35f), vec3(0.22f, 0.06f, 0.30f), -8.0f), Surface::colour(vec3(0.15f, 0.20f, 0.45f)));
	addProp("desk_clutter", Shape::Cube, Xform::boxRotY(vec3(-3.66f, 0.875f, -4.34f), vec3(0.20f, 0.05f, 0.27f), 5.0f), Surface::colour(vec3(0.45f, 0.12f, 0.10f)));
	addProp("desk_clutter", Shape::Cylinder, Xform::box(vec3(-4.45f, 0.84f, -3.98f), vec3(0.08f, 0.10f, 0.08f)), Surface::colour(vec3(0.85f, 0.85f, 0.8f), 0.4f, 32.0f));

	// the note: the first clue
	addProp("note", Shape::Cube, Xform::boxRotY(vec3(-4.05f, 0.792f, -4.15f), vec3(0.21f, 0.004f, 0.28f), 12.0f), Surface::colour(PAPER, 0.05f, 4.0f));
	addProp("note", Shape::Cube, Xform::boxRotY(vec3(-4.05f, 0.795f, -4.22f), vec3(0.14f, 0.002f, 0.012f), 12.0f), Surface::colour(vec3(0.1f), 0.0f, 4.0f), false);
	addProp("note", Shape::Cube, Xform::boxRotY(vec3(-4.04f, 0.795f, -4.17f), vec3(0.15f, 0.002f, 0.012f), 12.0f), Surface::colour(vec3(0.1f), 0.0f, 4.0f), false);
	addProp("note", Shape::Cube, Xform::boxRotY(vec3(-4.04f, 0.795f, -4.12f), vec3(0.12f, 0.002f, 0.012f), 12.0f), Surface::colour(vec3(0.1f), 0.0f, 4.0f), false);
	setAnchor("note", glm::translate(glm::mat4(1.0f), vec3(-4.05f, 0.79f, -4.15f)));

	const std::string noteText =
		"DAY 41\n\n"
		"They changed every lock code again after the incident. My memory is going, so:\n\n"
		" - The CABINET combination is on the indicator board by the cot.\n"
		"   Count the lit lights in each row, top to bottom:\n"
		"   red, green, blue, yellow. Dead sockets don't count.\n\n"
		" - The cabinet keeps the key. The key opens the box on the table.\n\n"
		" - The EXIT code is inside the box.\n\n"
		"If you are reading this, I didn't make it out.\n"
		"You still can.\n\n"
		"                                        - H.";

	addInteractable<Readable>("note",
		AABB::fromCenterSize(vec3(-4.05f, 0.80f, -4.15f), vec3(0.30f, 0.05f, 0.32f)),
		"read note", "Note on the desk", noteText,
		[](GameContext& ctx) { ctx.state.hasReadNote = true; });
}

// --------------------------------------------------------------------------- B: cabinet + key

void Level::buildCabinet() {
	const vec3 lo(4.6f, 0.0f, -5.0f), hi(5.6f, 1.9f, -4.4f);
	setAnchor("cabinet", glm::translate(glm::mat4(1.0f), vec3(5.1f, 0.0f, -4.7f)));

	// PLACEHOLDER: hollow cabinet body (back, sides, top, bottom, shelf)
	Surface body = metal(STEEL, 0.5f);
	Surface inside = Surface::colour(vec3(0.12f, 0.13f, 0.14f), 0.2f, 16.0f);
	addProp("cabinet", Shape::Cube, Xform::span(vec3(lo.x, 0.0f, lo.z), vec3(hi.x, hi.y, lo.z + 0.04f)), body);
	addProp("cabinet", Shape::Cube, Xform::span(vec3(lo.x, 0.0f, lo.z + 0.04f), vec3(lo.x + 0.04f, hi.y, hi.z - 0.02f)), body);
	addProp("cabinet", Shape::Cube, Xform::span(vec3(hi.x - 0.04f, 0.0f, lo.z + 0.04f), vec3(hi.x, hi.y, hi.z - 0.02f)), body);
	addProp("cabinet", Shape::Cube, Xform::span(vec3(lo.x, hi.y - 0.04f, lo.z + 0.04f), vec3(hi.x, hi.y, hi.z - 0.02f)), body);
	addProp("cabinet", Shape::Cube, Xform::span(vec3(lo.x, 0.0f, lo.z + 0.04f), vec3(hi.x, 0.08f, hi.z - 0.02f)), body);
	addProp("cabinet", Shape::Cube, Xform::span(vec3(lo.x + 0.04f, 0.04f, lo.z + 0.04f), vec3(hi.x - 0.04f, hi.y - 0.04f, lo.z + 0.05f)), inside);
	addProp("cabinet", Shape::Cube, Xform::span(vec3(lo.x + 0.04f, 0.90f, lo.z + 0.05f), vec3(hi.x - 0.04f, 0.93f, hi.z - 0.03f)), inside);
	addCollider(AABB(lo, hi), "cabinet");

	// door: hinge on the west edge, swings out towards the room (+Z)
	auto doorPtr = std::unique_ptr<Door>(new Door("cabinet_door", vec3(lo.x, 0.0f, hi.z + 0.005f),
		vec3(0.0f, 1.0f, 0.0f), -105.0f, 120.0f));
	Door& door = addDoor(std::move(doorPtr));
	door.setPanel(*this, Xform::span(vec3(0.0f, 0.01f, -0.015f), vec3(1.0f, 1.89f, 0.015f)), body, true, false);
	door.addPart(*this, Shape::Cube, Xform::span(vec3(0.90f, 0.80f, 0.015f), vec3(0.93f, 1.05f, 0.05f)), metal(CHROME, 0.9f));
	door.addPart(*this, Shape::Cube, Xform::span(vec3(0.68f, 1.15f, 0.015f), vec3(0.88f, 1.42f, 0.035f)), metal(DARK_STEEL, 0.3f));
	for (int r = 0; r < 3; r++)
		for (int c = 0; c < 3; c++)
			door.addPart(*this, Shape::Cube,
				Xform::box(vec3(0.72f + c * 0.06f, 1.20f + r * 0.055f, 0.038f), vec3(0.04f, 0.035f, 0.01f)),
				Surface::colour(vec3(0.6f), 0.4f, 32.0f), false);
	int led = door.addPart(*this, Shape::Sphere, Xform::box(vec3(0.78f, 1.38f, 0.04f), vec3(0.03f)),
		Surface::glow(vec3(1.0f, 0.1f, 0.05f)), false);

	addInteractable<Cabinet>(AABB(lo, vec3(hi.x, hi.y, hi.z + 0.03f)), door, led);

	// PLACEHOLDER: key on the shelf (pieces in key space, rotated / bobbed by Key::update)
	const vec3 keyPos(5.1f, 1.02f, -4.68f);
	Surface brass = metal(vec3(0.85f, 0.62f, 0.18f), 0.9f);
	brass.emissive = vec3(0.12f, 0.08f, 0.01f);	// a little glow so it reads in the dark cabinet
	Key& key = addInteractable<Key>(keyPos, door);

	const glm::mat4 bowLocal = Xform::cylinderAlong(vec3(-0.09f, 0.0f, 0.0f), 0.02f, 0.10f, 'z');
	const glm::mat4 shaftLocal = Xform::span(vec3(-0.045f, -0.01f, -0.008f), vec3(0.12f, 0.01f, 0.008f));
	const glm::mat4 tooth1 = Xform::span(vec3(0.06f, -0.04f, -0.008f), vec3(0.08f, -0.01f, 0.008f));
	const glm::mat4 tooth2 = Xform::span(vec3(0.095f, -0.032f, -0.008f), vec3(0.12f, -0.01f, 0.008f));
	key.addPiece(addProp("key", Shape::Cylinder, glm::translate(glm::mat4(1.0f), keyPos) * bowLocal, brass), bowLocal);
	key.addPiece(addProp("key", Shape::Cube, glm::translate(glm::mat4(1.0f), keyPos) * shaftLocal, brass), shaftLocal);
	key.addPiece(addProp("key", Shape::Cube, glm::translate(glm::mat4(1.0f), keyPos) * tooth1, brass), tooth1);
	key.addPiece(addProp("key", Shape::Cube, glm::translate(glm::mat4(1.0f), keyPos) * tooth2, brass), tooth2);
	setAnchor("key", glm::translate(glm::mat4(1.0f), keyPos));
}

// --------------------------------------------------------------------------- C: table + lockbox

void Level::buildTable() {
	const vec3 lo(-0.75f, 0.0f, -0.15f), hi(0.75f, 0.77f, 0.75f);
	setAnchor("table", glm::translate(glm::mat4(1.0f), vec3(0.0f, 0.0f, 0.3f)));

	// PLACEHOLDER: table
	addProp("table", Shape::Cube, Xform::span(vec3(lo.x, 0.72f, lo.z), hi), woodSurface(0.85f));
	const float legX[2] = { lo.x + 0.06f, hi.x - 0.06f };
	const float legZ[2] = { lo.z + 0.06f, hi.z - 0.06f };
	for (float x : legX)
		for (float z : legZ)
			addProp("table", Shape::Cube, Xform::box(vec3(x, 0.36f, z), vec3(0.07f, 0.72f, 0.07f)), woodSurface(0.6f));
	addCollider(AABB(lo, hi), "table");

	// PLACEHOLDER: lockbox (hollow so the card inside is visible once the lid lifts)
	const vec3 bLo(-0.2f, 0.77f, 0.15f), bHi(0.2f, 0.93f, 0.45f);
	setAnchor("lockbox_body", glm::translate(glm::mat4(1.0f), vec3(0.0f, 0.77f, 0.3f)));
	Surface boxMetal = metal(vec3(0.22f, 0.26f, 0.22f), 0.5f);
	const float w = 0.015f;
	addProp("lockbox_body", Shape::Cube, Xform::span(bLo, vec3(bHi.x, bLo.y + w, bHi.z)), boxMetal);
	addProp("lockbox_body", Shape::Cube, Xform::span(bLo, vec3(bLo.x + w, bHi.y, bHi.z)), boxMetal);
	addProp("lockbox_body", Shape::Cube, Xform::span(vec3(bHi.x - w, bLo.y, bLo.z), bHi), boxMetal);
	addProp("lockbox_body", Shape::Cube, Xform::span(bLo, vec3(bHi.x, bHi.y, bLo.z + w)), boxMetal);
	addProp("lockbox_body", Shape::Cube, Xform::span(vec3(bLo.x, bLo.y, bHi.z - w), bHi), boxMetal);
	addProp("lockbox_body", Shape::Cylinder, Xform::cylinderAlong(vec3(0.0f, 0.86f, bHi.z + 0.004f), 0.01f, 0.035f, 'z'), metal(vec3(0.75f, 0.6f, 0.2f), 0.9f));
	addProp("lockbox_body", Shape::Cube, Xform::box(vec3(0.0f, 0.855f, bHi.z + 0.01f), vec3(0.006f, 0.018f, 0.004f)), Surface::colour(vec3(0.02f)), false);

	// lid: hinge along the back top edge, lifts up and back
	auto lidPtr = std::unique_ptr<Door>(new Door("lockbox", vec3(0.0f, bHi.y, bLo.z),
		vec3(1.0f, 0.0f, 0.0f), -105.0f, 140.0f));
	Door& lid = addDoor(std::move(lidPtr));
	lid.setPanel(*this, Xform::span(vec3(-0.205f, 0.0f, -0.005f), vec3(0.205f, 0.03f, 0.305f)), boxMetal, false, false);
	lid.addPart(*this, Shape::Cube, Xform::span(vec3(-0.04f, -0.02f, 0.30f), vec3(0.04f, 0.03f, 0.31f)), metal(CHROME));

	addInteractable<LockBox>(AABB(bLo, vec3(bHi.x, bHi.y + 0.03f, bHi.z + 0.01f)), lid);

	// the card inside the box (made readable in Level::build)
	addProp("exit_card", Shape::Cube, Xform::boxRotY(vec3(0.0f, 0.79f, 0.30f), vec3(0.20f, 0.004f, 0.12f), 8.0f), Surface::colour(PAPER, 0.05f, 4.0f), false);
	addProp("exit_card", Shape::Cube, Xform::boxRotY(vec3(0.0f, 0.793f, 0.27f), vec3(0.16f, 0.002f, 0.02f), 8.0f), Surface::colour(vec3(0.7f, 0.05f, 0.05f)), false);
	addProp("exit_card", Shape::Cube, Xform::boxRotY(vec3(0.0f, 0.793f, 0.32f), vec3(0.12f, 0.002f, 0.015f), 8.0f), Surface::colour(vec3(0.1f)), false);
	setAnchor("exit_card", glm::translate(glm::mat4(1.0f), vec3(0.0f, 0.79f, 0.30f)));

	// PLACEHOLDER: table clutter
	addProp("table_clutter", Shape::Cylinder, Xform::box(vec3(0.50f, 0.82f, 0.10f), vec3(0.08f, 0.10f, 0.08f)), Surface::colour(vec3(0.2f, 0.3f, 0.5f), 0.4f, 32.0f));
	addProp("table_clutter", Shape::Cube, Xform::boxRotY(vec3(-0.45f, 0.775f, 0.45f), vec3(0.30f, 0.01f, 0.40f), -15.0f), Surface::colour(vec3(0.75f, 0.72f, 0.62f), 0.05f, 4.0f));
	addProp("table_clutter", Shape::Cube, Xform::box(vec3(0.45f, 0.83f, 0.55f), vec3(0.22f, 0.12f, 0.12f)), metal(vec3(0.15f, 0.15f, 0.12f), 0.3f));
	addProp("table_clutter", Shape::Cylinder, Xform::box(vec3(0.52f, 0.97f, 0.55f), vec3(0.01f, 0.18f, 0.01f)), metal(CHROME));
	setAnchor("table_clutter", glm::mat4(1.0f));
}

// --------------------------------------------------------------------------- D: clue board

void Level::buildClueBoard(const PuzzleManager& puzzles) {
	const float wallX = ROOM_MIN_X;
	const vec3 lo(wallX, 1.0f, 1.6f), hi(wallX + 0.05f, 2.1f, 3.2f);
	setAnchor("clueboard", glm::translate(glm::mat4(1.0f), vec3(wallX, 1.0f, 2.4f)));

	// PLACEHOLDER: board + frame
	addProp("clueboard", Shape::Cube, Xform::span(vec3(wallX, 0.95f, 1.55f), vec3(wallX + 0.03f, 2.15f, 3.25f)), metal(DARK_STEEL, 0.4f));
	addProp("clueboard", Shape::Cube, Xform::span(lo, hi), Surface::colour(vec3(0.07f, 0.08f, 0.09f), 0.3f, 32.0f));

	// four rows of indicator lights; lit count per row = one digit
	const auto& rows = puzzles.clueRows();
	const float rowY[4] = { 1.90f, 1.66f, 1.42f, 1.18f };
	const float x = hi.x + 0.02f;
	for (int r = 0; r < 4; r++) {
		// colour label on the left (seen from the room, +Z is to the viewer's left)
		addProp("clueboard", Shape::Cube, Xform::box(vec3(hi.x + 0.005f, rowY[r], 3.0f), vec3(0.01f, 0.13f, 0.18f)),
			Surface::glow(rows[r].colour, 0.55f), false);

		for (int i = 0; i < 5; i++) {
			vec3 p(x, rowY[r], 2.66f - i * 0.22f);	// read left to right
			if (i < rows[r].count)
				addProp("clueboard", Shape::Sphere, Xform::box(p, vec3(0.11f)), Surface::glow(rows[r].colour, 1.5f), false);
			else
				addProp("clueboard", Shape::Sphere, Xform::box(p, vec3(0.09f)), Surface::colour(vec3(0.05f), 0.5f, 64.0f), false);
		}
	}

	addInteractable<ClueBoard>(AABB(lo, vec3(hi.x + 0.08f, hi.y, hi.z)));
}

// --------------------------------------------------------------------------- E: exit door + keypad

void Level::buildExit() {
	// door: hinge at the west side of the doorway, swings out into the corridor (+Z)
	auto doorPtr = std::unique_ptr<Door>(new Door("exit_door", vec3(3.4f, 0.0f, ROOM_MAX_Z + 0.15f),
		vec3(0.0f, 1.0f, 0.0f), -95.0f, 75.0f));
	Door& door = addDoor(std::move(doorPtr));
	door.setPanel(*this, Xform::span(vec3(0.0f, 0.0f, -0.05f), vec3(1.2f, 2.19f, 0.05f)), metal(vec3(0.20f, 0.22f, 0.25f), 0.5f), true, true);
	// room-side details: handle, window slot, hazard stripes
	door.addPart(*this, Shape::Cube, Xform::span(vec3(1.00f, 0.95f, -0.11f), vec3(1.12f, 1.00f, -0.05f)), metal(CHROME, 0.9f));
	door.addPart(*this, Shape::Cube, Xform::span(vec3(0.35f, 1.55f, -0.06f), vec3(0.85f, 1.85f, -0.05f)), Surface::colour(vec3(0.03f, 0.04f, 0.05f), 0.9f, 128.0f));
	for (int i = 0; i < 6; i++)
		door.addPart(*this, Shape::Cube,
			Xform::span(vec3(0.05f + i * 0.19f, 0.05f, -0.06f), vec3(0.15f + i * 0.19f, 0.20f, -0.05f)),
			Surface::colour(HAZARD, 0.2f, 16.0f));
	exitDoor = &door;
	addInteractable<ExitDoor>(door);

	// keypad panel on the wall, east of the door
	const vec3 kp(5.05f, 1.35f, ROOM_MAX_Z);
	setAnchor("exit_keypad", glm::translate(glm::mat4(1.0f), kp));
	addProp("exit_keypad", Shape::Cube, Xform::box(kp - vec3(0.0f, 0.0f, 0.03f), vec3(0.24f, 0.36f, 0.06f)), metal(DARK_STEEL, 0.4f));
	int screen = addProp("exit_keypad", Shape::Cube, Xform::box(kp + vec3(0.0f, 0.11f, -0.065f), vec3(0.17f, 0.07f, 0.01f)),
		Surface::glow(vec3(1.0f, 0.1f, 0.05f)), false);
	for (int r = 0; r < 4; r++)
		for (int c = 0; c < 3; c++)
			addProp("exit_keypad", Shape::Cube,
				Xform::box(kp + vec3(-0.06f + c * 0.06f, 0.03f - r * 0.05f, -0.065f), vec3(0.045f, 0.035f, 0.01f)),
				Surface::colour(vec3(0.55f), 0.5f, 32.0f), false);
	addInteractable<ExitKeypadPanel>(AABB::fromCenterSize(kp - vec3(0.0f, 0.0f, 0.05f), vec3(0.30f, 0.42f, 0.12f)), screen);

	// EXIT sign above the door
	addProp("room", Shape::Cube, Xform::box(vec3(4.0f, 2.55f, ROOM_MAX_Z - 0.03f), vec3(0.45f, 0.15f, 0.05f)),
		Surface::glow(vec3(0.15f, 1.0f, 0.35f), 1.1f), false);
}

// --------------------------------------------------------------------------- decor

void Level::buildDecor() {
	// PLACEHOLDER: cot where the player wakes up
	setAnchor("cot", glm::translate(glm::mat4(1.0f), vec3(-2.6f, 0.0f, 4.52f)));
	Surface cotFrame = metal(vec3(0.25f, 0.28f, 0.24f), 0.4f);
	addProp("cot", Shape::Cube, Xform::span(vec3(-3.6f, 0.30f, 4.1f), vec3(-1.6f, 0.38f, 4.95f)), cotFrame);
	addProp("cot", Shape::Cube, Xform::span(vec3(-3.55f, 0.38f, 4.15f), vec3(-1.65f, 0.50f, 4.90f)), Surface::colour(FABRIC, 0.05f, 4.0f));
	addProp("cot", Shape::Cube, Xform::span(vec3(-3.5f, 0.50f, 4.25f), vec3(-3.05f, 0.58f, 4.80f)), Surface::colour(vec3(0.75f, 0.74f, 0.70f), 0.05f, 4.0f));
	addProp("cot", Shape::Cube, Xform::boxRotY(vec3(-2.2f, 0.52f, 4.45f), vec3(0.9f, 0.03f, 0.75f), 6.0f), Surface::colour(vec3(0.30f, 0.22f, 0.15f), 0.05f, 4.0f));
	const float cotX[2] = { -3.55f, -1.65f };
	const float cotZ[2] = { 4.15f, 4.90f };
	for (float x : cotX)
		for (float z : cotZ)
			addProp("cot", Shape::Cube, Xform::box(vec3(x, 0.15f, z), vec3(0.05f, 0.30f, 0.05f)), cotFrame);
	addCollider(AABB(vec3(-3.6f, 0.0f, 4.1f), vec3(-1.6f, 0.58f, 4.95f)), "cot");

	// PLACEHOLDER: backpack next to the cot (try USE_IMPORTED_MODELS)
	setAnchor("backpack", glm::translate(glm::mat4(1.0f), vec3(-1.25f, 0.0f, 4.65f)));
	addProp("backpack", Shape::Cube, Xform::boxRotY(vec3(-1.25f, 0.25f, 4.65f), vec3(0.35f, 0.5f, 0.22f), -20.0f), Surface::colour(vec3(0.25f, 0.30f, 0.15f), 0.05f, 4.0f));
	addProp("backpack", Shape::Cube, Xform::boxRotY(vec3(-1.22f, 0.18f, 4.53f), vec3(0.25f, 0.2f, 0.06f), -20.0f), Surface::colour(vec3(0.20f, 0.25f, 0.12f), 0.05f, 4.0f));
	addCollider(AABB(vec3(-1.5f, 0.0f, 4.45f), vec3(-1.0f, 0.5f, 4.85f)), "backpack");

	// PLACEHOLDER: barrels along the east wall
	Surface barrel = Surface::colour(RUST, 0.3f, 16.0f);
	Surface band = metal(vec3(0.2f, 0.2f, 0.2f), 0.4f);
	const vec3 barrels[3] = { vec3(5.45f, 0.0f, 1.4f), vec3(5.45f, 0.0f, 2.1f), vec3(4.80f, 0.0f, 1.75f) };
	for (int i = 0; i < 3; i++) {
		const vec3& b = barrels[i];
		addProp("barrels", Shape::Cylinder, Xform::box(b + vec3(0.0f, 0.45f, 0.0f), vec3(0.6f, 0.9f, 0.6f)), barrel);
		addProp("barrels", Shape::Cylinder, Xform::box(b + vec3(0.0f, 0.25f, 0.0f), vec3(0.62f, 0.04f, 0.62f)), band);
		addProp("barrels", Shape::Cylinder, Xform::box(b + vec3(0.0f, 0.68f, 0.0f), vec3(0.62f, 0.04f, 0.62f)), band);
		addCollider(AABB(b + vec3(-0.3f, 0.0f, -0.3f), b + vec3(0.3f, 0.9f, 0.3f)), "barrel");
	}
	setAnchor("barrels", glm::mat4(1.0f));

	// PLACEHOLDER: crates along the west wall
	setAnchor("crates", glm::mat4(1.0f));
	Surface crate = Surface::textured(TextureId::Metal, glm::vec2(1.0f));
	crate.worldUV = false;		// one texture per face
	const glm::mat4 c1 = Xform::box(vec3(-5.5f, 0.4f, -1.5f), vec3(0.8f));
	const glm::mat4 c2 = Xform::boxRotY(vec3(-5.45f, 0.3f, -0.55f), vec3(0.6f), 18.0f);
	const glm::mat4 c3 = Xform::boxRotY(vec3(-5.5f, 1.05f, -1.5f), vec3(0.5f), -10.0f);
	addProp("crates", Shape::Cube, c1, crate);
	addProp("crates", Shape::Cube, c2, crate);
	addProp("crates", Shape::Cube, c3, crate);
	addCollider(Collision::transformUnitCube(c1), "crate");
	addCollider(Collision::transformUnitCube(c2), "crate");

	// pipes along the ceiling (above head height, no collision)
	Surface pipe = metal(vec3(0.28f, 0.30f, 0.30f), 0.6f);
	addProp("pipes", Shape::Cylinder, Xform::cylinderAlong(vec3(0.0f, 2.80f, -4.75f), 12.0f, 0.14f, 'x'), pipe);
	addProp("pipes", Shape::Cylinder, Xform::cylinderAlong(vec3(0.0f, 2.62f, -4.80f), 12.0f, 0.08f, 'x'), metal(vec3(0.45f, 0.18f, 0.10f), 0.5f));
	addProp("pipes", Shape::Cylinder, Xform::cylinderAlong(vec3(5.75f, 2.80f, 0.0f), 10.0f, 0.14f, 'z'), pipe);
	addProp("pipes", Shape::Cylinder, Xform::box(vec3(5.75f, 1.4f, -4.75f), vec3(0.14f, 2.8f, 0.14f)), pipe);
	setAnchor("pipes", glm::mat4(1.0f));
}

// --------------------------------------------------------------------------- lights

void Level::buildLights() {
	// ceiling strip light (main light, shadow casting, slightly unreliable)
	addProp("ceiling_lamp", Shape::Cube, Xform::box(vec3(0.0f, 2.96f, 0.3f), vec3(1.3f, 0.08f, 0.32f)), metal(DARK_STEEL, 0.3f), false);
	int tube = addProp("ceiling_lamp", Shape::Cube, Xform::box(vec3(0.0f, 2.91f, 0.3f), vec3(1.2f, 0.03f, 0.18f)),
		Surface::glow(vec3(1.0f, 0.95f, 0.85f), 1.0f), false);
	setAnchor("ceiling_lamp", glm::translate(glm::mat4(1.0f), vec3(0.0f, 3.0f, 0.3f)));

	LevelLight ceiling;
	ceiling.position = vec3(0.0f, 2.7f, 0.3f);
	ceiling.ambient = vec3(0.07f, 0.07f, 0.08f);
	ceiling.diffuse = vec3(1.0f, 0.93f, 0.80f) * 0.95f;
	ceiling.specular = vec3(0.5f);
	ceiling.k1 = 0.07f;
	ceiling.k2 = 0.022f;
	ceiling.castsShadow = true;
	ceiling.flicker = Flicker::Fluorescent;
	ceiling.fixtureProp = tube;
	lights.push_back(ceiling);

	// desk lamp: warm pool of light on the note
	int bulb = addProp("desk_lamp", Shape::Sphere, Xform::box(vec3(-4.5f, 1.07f, -4.4f), vec3(0.07f)),
		Surface::glow(vec3(1.0f, 0.8f, 0.5f), 1.5f), false);
	LevelLight desk;
	desk.position = vec3(-4.5f, 1.0f, -4.35f);
	desk.ambient = vec3(0.0f);
	desk.diffuse = vec3(1.0f, 0.75f, 0.42f) * 1.3f;
	desk.specular = vec3(0.6f, 0.5f, 0.3f);
	desk.k1 = 0.35f;
	desk.k2 = 0.45f;
	desk.fixtureProp = bulb;
	lights.push_back(desk);

	// red emergency light above the exit
	int beacon = addProp("room", Shape::Cube, Xform::box(vec3(4.0f, 2.38f, ROOM_MAX_Z - 0.05f), vec3(0.32f, 0.10f, 0.10f)),
		Surface::glow(vec3(1.0f, 0.08f, 0.04f), 1.0f), false);
	LevelLight emergency;
	emergency.position = vec3(4.0f, 2.3f, 4.7f);
	emergency.ambient = vec3(0.02f, 0.0f, 0.0f);
	emergency.diffuse = vec3(1.0f, 0.10f, 0.05f) * 1.2f;
	emergency.specular = vec3(0.6f, 0.1f, 0.1f);
	emergency.k1 = 0.25f;
	emergency.k2 = 0.22f;
	emergency.flicker = Flicker::Pulse;
	emergency.fixtureProp = beacon;
	lights.push_back(emergency);

	// cool light in the exit corridor, visible once the door opens
	LevelLight corridor;
	corridor.position = vec3(4.0f, 2.3f, 7.7f);
	corridor.ambient = vec3(0.02f, 0.025f, 0.03f);
	corridor.diffuse = vec3(0.70f, 0.90f, 1.0f) * 1.2f;
	corridor.specular = vec3(0.4f);
	corridor.k1 = 0.45f;
	corridor.k2 = 0.80f;
	lights.push_back(corridor);
}

// --------------------------------------------------------------------------- imported models

void Level::applyModelReplacements() {
	/*
		Swap any placeholder group for an imported model. The model is drawn at
		anchor(group) * local; collision and interaction volumes are unchanged.
		Paths are relative to assets/models (copied next to the executable).

		Examples once you have the assets:
			replaceWithModel("desk",    "desk/scene.gltf",    glm::scale(glm::mat4(1.0f), glm::vec3(0.01f)));
			replaceWithModel("cabinet", "cabinet/scene.gltf", glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0, 1, 0)));
			replaceWithModel("key",     "key/key.obj",        glm::scale(glm::mat4(1.0f), glm::vec3(0.05f)));
	*/
	if (!USE_IMPORTED_MODELS)
		return;

	// A model that already ships with the engine repo. The engine's Model loader ignores glTF
	// node transforms, so the raw (Z-up, ~6 units tall) mesh is rotated upright and scaled to ~0.5 m.
	glm::mat4 backpack = glm::rotate(glm::mat4(1.0f), glm::radians(-20.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	backpack = glm::translate(backpack, glm::vec3(0.0f, 0.29f, 0.0f));
	backpack = glm::rotate(backpack, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	backpack = glm::scale(backpack, glm::vec3(0.08f));
	replaceWithModel("backpack", "survival_guitar_backpack/scene.gltf", backpack);
}
