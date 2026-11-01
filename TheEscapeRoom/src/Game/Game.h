#ifndef ESCAPE_GAME_H
#define ESCAPE_GAME_H

#include <deque>
#include <string>
#include <vector>

#include "graphics/scene.h"		// engine window / ImGui front-end / camera matrices

#include "GameState.h"
#include "GameContext.h"
#include "Level.h"
#include "../Player/Player.h"
#include "../Interaction/InteractionSystem.h"
#include "../Puzzles/PuzzleManager.h"
#include "../Objects/Keypad.h"
#include "../Rendering/WorldRenderer.h"
#include "../UI/GameUI.h"

struct GameSettings {
	bool timed = false;
	int timeLimitMinutes = 8;
	float mouseSensitivity = 0.10f;
	bool invertY = false;
	bool headBob = true;
	float fov = 62.0f;
	bool shadows = true;
	bool torchOn = true;
	bool showColliders = false;		// F2
	bool showDebug = false;			// F1
};

struct Toast {
	std::string text;
	float age = 0.0f;
};

/*
	The Escape Room game. Owns the rules-side objects and runs the loop described in the plan:

		1 read input  2 move player  3 collide  4 find interaction target  5 process requests
		6 update puzzle / game state  7 animations & timers  8 render 3D  9 render UI
*/
class Game {
public:
	explicit Game(Scene& scene);

	bool init();
	void frame(float dt);
	void cleanup();

	// ---- actions (called by the UI) -----------------------------------------------------
	void startNewGame();
	void resume();
	void pause();
	void quitToMenu();
	void quitGame();

	void keypadDigit(int digit);
	void keypadBackspace();
	void keypadClear();
	void keypadSubmit();
	void closeKeypad();
	void closeReadable();

	// ---- read access for the UI -----------------------------------------------------------
	GameScreen screen() const { return currentScreen; }
	const GameState& state() const { return gameState; }
	const PuzzleManager& puzzles() const { return puzzleManager; }
	const Keypad& keypad() const { return keypadLogic; }
	const Level& level() const { return world; }
	const Player& player() const { return thePlayer; }
	GameSettings& settings() { return gameSettings; }

	const std::string& prompt() const { return currentPrompt; }
	const std::deque<Toast>& toasts() const { return toastList; }
	const std::string& readableTitle() const { return readTitle; }
	const std::string& readableBody() const { return readBody; }
	std::string objective() const;
	float screenTime() const { return screenTimer; }
	float fps() const { return smoothedFps; }
	int drawCalls() const { return renderer.drawCalls(); }
	bool exitDoorOpen() const { return world.exitDoor && world.exitDoor->isOpeningOrOpen(); }

private:
	struct InputFrame {
		bool escape = false;
		bool interact = false;
		bool torch = false;
		bool debug = false;
		bool colliders = false;
		bool pauseButton = false;	// gamepad start
	};

	Scene& scene;
	WorldRenderer renderer;
	GameUI ui;

	Level world;
	Player thePlayer;
	GameState gameState;
	PuzzleManager puzzleManager;
	InteractionSystem interaction;
	Keypad keypadLogic;
	UIRequests requests;
	GameSettings gameSettings;

	GameScreen currentScreen = GameScreen::MainMenu;
	float screenTimer = 0.0f;
	float clock = 0.0f;
	float smoothedFps = 60.0f;

	std::string currentPrompt;
	std::deque<Toast> toastList;
	std::string readTitle;
	std::string readBody;

	float flash = 0.0f;
	glm::vec3 flashColour = glm::vec3(1.0f, 0.0f, 0.0f);
	float exitFade = 0.0f;

	bool cursorCaptured = false;
	int lookSuppressFrames = 0;
	bool padInteractWasDown = false;
	bool padStartWasDown = false;

	GameContext context();
	InputFrame readInput();
	PlayerInput readMovement();
	void setScreen(GameScreen s);
	void newRound();
	void updatePlaying(float dt, const InputFrame& in);
	void updateWorld(float dt);
	void processRequests();
	void checkEndConditions(float dt);
	void updateCursor();
	void renderWorld();
	void toast(const std::string& text);
};

#endif // !ESCAPE_GAME_H
