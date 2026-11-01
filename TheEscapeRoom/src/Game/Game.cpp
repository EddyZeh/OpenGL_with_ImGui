#include "Game.h"

#include <glm/gtc/matrix_transform.hpp>

#include <chrono>
#include <cmath>
#include <random>

#include "io/Keyboard.h"	// engine input
#include "io/Mouse.h"

Game::Game(Scene& scene) : scene(scene) {}

GameContext Game::context() {
	return GameContext{ gameState, puzzleManager, world, requests };
}

// =========================================================================== lifetime

bool Game::init() {
	if (!renderer.init())
		return false;

	ui.init();

	// build a room straight away so the main menu has something to show behind it
	newRound();
	renderer.setupLights(world);
	setScreen(GameScreen::MainMenu);
	return true;
}

void Game::cleanup() {
	world.clear();
	renderer.cleanup();
}

void Game::newRound() {
	unsigned int seed = (unsigned int)std::chrono::high_resolution_clock::now().time_since_epoch().count();
	seed ^= std::random_device{}();
	puzzleManager.generate(seed);

	world.build(puzzleManager);

	gameState = GameState();
	gameState.timed = gameSettings.timed;
	gameState.timeLimit = gameSettings.timeLimitMinutes * 60.0f;

	thePlayer.spawn(world.playerSpawn, world.spawnYaw, -5.0f);
	interaction.clearTarget();
	keypadLogic.begin(KeypadTarget::Cabinet);
	requests.clear();
	toastList.clear();
	currentPrompt.clear();
	flash = 0.0f;
	exitFade = 0.0f;
}

// =========================================================================== UI actions

void Game::startNewGame() {
	newRound();
	setScreen(GameScreen::Playing);
	toast("You wake up on a cot. The only door is sealed.");
	toast("Find a way out. (E to interact, F for the torch)");
}

void Game::resume() {
	if (currentScreen == GameScreen::Paused)
		setScreen(GameScreen::Playing);
}

void Game::pause() {
	if (currentScreen == GameScreen::Playing)
		setScreen(GameScreen::Paused);
}

void Game::quitToMenu() {
	newRound();
	setScreen(GameScreen::MainMenu);
}

void Game::quitGame() {
	scene.setShouldClose(true);
}

void Game::keypadDigit(int digit) { keypadLogic.pressDigit(digit); }
void Game::keypadBackspace() { keypadLogic.backspace(); }
void Game::keypadClear() { keypadLogic.clearEntry(); }

void Game::keypadSubmit() {
	CodeResult r = keypadLogic.submit(puzzleManager, gameState);
	if (r == CodeResult::Accepted) {
		flash = 0.18f;
		flashColour = glm::vec3(0.1f, 1.0f, 0.3f);
	}
	else if (r == CodeResult::Rejected) {
		flash = 0.30f;
		flashColour = glm::vec3(1.0f, 0.0f, 0.0f);
	}
}

void Game::closeKeypad() {
	if (currentScreen == GameScreen::Keypad)
		setScreen(GameScreen::Playing);
}

void Game::closeReadable() {
	if (currentScreen == GameScreen::Reading)
		setScreen(GameScreen::Playing);
}

void Game::setScreen(GameScreen s) {
	currentScreen = s;
	screenTimer = 0.0f;
	if (s != GameScreen::Playing)
		currentPrompt.clear();
}

void Game::toast(const std::string& text) {
	toastList.push_back({ text, 0.0f });
	while (toastList.size() > 4)
		toastList.pop_front();
}

// =========================================================================== input

Game::InputFrame Game::readInput() {
	// Edge-triggered keys come from ImGui's input queue: it trickles events, so a quick tap
	// (press + release inside one frame) still registers. The engine's Keyboard::keyWentDown
	// would miss it. Held keys (movement) still use the engine Keyboard.
	InputFrame in;
	in.escape = ImGui::IsKeyPressed(ImGuiKey_Escape, false);
	in.interact = ImGui::IsKeyPressed(ImGuiKey_E, false);
	in.torch = ImGui::IsKeyPressed(ImGuiKey_F, false);
	in.debug = ImGui::IsKeyPressed(ImGuiKey_F1, false);
	in.colliders = ImGui::IsKeyPressed(ImGuiKey_F2, false);

	// gamepad (engine Joystick, raw XInput-style layout): A = interact, Start = pause
	Joystick& pad = scene.mainJ;
	pad.update();
	if (pad.isPresent()) {
		bool a = pad.getButtonCount() > GLFW_GAMEPAD_BUTTON_A && pad.buttonState(GLFW_GAMEPAD_BUTTON_A) == GLFW_PRESS;
		bool start = pad.getButtonCount() > 7 && pad.buttonState(7) == GLFW_PRESS;
		in.interact = in.interact || (a && !padInteractWasDown);
		in.pauseButton = start && !padStartWasDown;
		padInteractWasDown = a;
		padStartWasDown = start;
	}
	return in;
}

PlayerInput Game::readMovement() {
	PlayerInput p;
	if (Keyboard::key(GLFW_KEY_W) || Keyboard::key(GLFW_KEY_UP))    p.forward += 1.0f;
	if (Keyboard::key(GLFW_KEY_S) || Keyboard::key(GLFW_KEY_DOWN))  p.forward -= 1.0f;
	if (Keyboard::key(GLFW_KEY_D) || Keyboard::key(GLFW_KEY_RIGHT)) p.right += 1.0f;
	if (Keyboard::key(GLFW_KEY_A) || Keyboard::key(GLFW_KEY_LEFT))  p.right -= 1.0f;
	p.sprint = Keyboard::key(GLFW_KEY_LEFT_SHIFT);

	// always drain the mouse so deltas never pile up while a menu is open
	double dx = Mouse::getDX();
	double dy = Mouse::getDY();
	Mouse::getScrollDY();
	if (lookSuppressFrames > 0) {
		lookSuppressFrames--;
		dx = dy = 0.0;
	}
	p.lookDX = dx;
	p.lookDY = gameSettings.invertY ? -dy : dy;

	// gamepad sticks
	Joystick& pad = scene.mainJ;
	if (pad.isPresent() && pad.getAxesCount() >= 4) {
		auto dead = [](float v) { return std::abs(v) > 0.2f ? v : 0.0f; };
		p.right += dead(pad.axesState(GLFW_GAMEPAD_AXIS_LEFT_X));
		p.forward -= dead(pad.axesState(GLFW_GAMEPAD_AXIS_LEFT_Y));
		p.lookDX += dead(pad.axesState(GLFW_GAMEPAD_AXIS_RIGHT_X)) * 12.0;
		p.lookDY -= dead(pad.axesState(GLFW_GAMEPAD_AXIS_RIGHT_Y)) * 12.0;
	}
	return p;
}

// =========================================================================== frame

void Game::frame(float dt) {
	clock += dt;
	screenTimer += dt;
	if (dt > 0.0f)
		smoothedFps += (1.0f / dt - smoothedFps) * 0.05f;

	// 1. input
	InputFrame in = readInput();

	if (in.debug) gameSettings.showDebug = !gameSettings.showDebug;
	if (in.colliders) gameSettings.showColliders = !gameSettings.showColliders;

	switch (currentScreen) {
	case GameScreen::Playing:
		if (in.escape || in.pauseButton) {
			pause();
			readMovement();		// drain
		}
		else {
			updatePlaying(dt, in);
		}
		break;
	case GameScreen::Paused:
		readMovement();
		if (in.escape || in.pauseButton)
			resume();
		break;
	case GameScreen::Keypad:
		readMovement();
		if (in.escape)
			closeKeypad();
		break;
	case GameScreen::Reading:
		readMovement();
		if (in.escape || in.interact)
			closeReadable();
		break;
	default:
		readMovement();
		break;
	}

	// 6-7. world, puzzle state and animations (frozen while paused / in menus)
	bool worldRuns = currentScreen == GameScreen::Playing || currentScreen == GameScreen::Keypad ||
		currentScreen == GameScreen::Reading || currentScreen == GameScreen::Victory;
	if (worldRuns)
		updateWorld(dt);

	// the clock keeps running while a keypad or note is open (timed mode adds pressure)
	if (currentScreen == GameScreen::Playing || currentScreen == GameScreen::Keypad || currentScreen == GameScreen::Reading)
		gameState.elapsed += dt;

	if (currentScreen == GameScreen::Keypad) {
		keypadLogic.update(dt);
		if (keypadLogic.wantsClose())
			closeKeypad();
	}

	checkEndConditions(dt);

	// toasts age; flash fades
	for (auto& t : toastList)
		t.age += dt;
	while (!toastList.empty() && toastList.front().age > 5.0f)
		toastList.pop_front();
	flash = (glm::max)(0.0f, flash - dt * 0.8f);

	updateCursor();

	// 8. 3D
	renderWorld();

	// 9. UI
	ui.draw(*this);
}

void Game::updatePlaying(float dt, const InputFrame& in) {
	if (in.torch)
		gameSettings.torchOn = !gameSettings.torchOn;

	// 2-3. move + collide
	thePlayer.lookSensitivity = gameSettings.mouseSensitivity;
	thePlayer.headBob = gameSettings.headBob;
	thePlayer.update(dt, readMovement(), world.colliders);

	// 4. what are we looking at?
	GameContext ctx = context();
	interaction.update(thePlayer.viewRay(), ctx);
	currentPrompt = interaction.target() ? interaction.target()->prompt(ctx) : std::string();

	// 5. interaction request -> the target decides
	if (in.interact)
		interaction.interact(ctx);

	processRequests();
}

void Game::updateWorld(float dt) {
	GameContext ctx = context();
	for (auto& obj : world.interactables)
		obj->update(dt, ctx);
	world.update(dt);

	// a door may have swung into the player
	if (currentScreen == GameScreen::Playing || currentScreen == GameScreen::Keypad)
		thePlayer.resolvePenetration(world.colliders);

	processRequests();
}

void Game::processRequests() {
	for (const auto& m : requests.messages)
		toast(m);

	if (requests.openKeypad && currentScreen == GameScreen::Playing) {
		keypadLogic.begin(requests.keypadTarget);
		setScreen(GameScreen::Keypad);
	}
	else if (requests.openReadable && currentScreen == GameScreen::Playing) {
		readTitle = requests.readableTitle;
		readBody = requests.readableBody;
		setScreen(GameScreen::Reading);
	}

	requests.clear();
}

void Game::checkEndConditions(float dt) {
	if (currentScreen == GameScreen::Playing && world.exitDoor && world.exitDoor->isOpeningOrOpen()) {
		if (thePlayer.bounds().intersects(world.exitZone)) {
			gameState.gameCompleted = true;
			setScreen(GameScreen::Victory);
		}
	}

	if (gameState.timed && !gameState.gameCompleted && !gameState.gameOver &&
		gameState.elapsed >= gameState.timeLimit &&
		(currentScreen == GameScreen::Playing || currentScreen == GameScreen::Keypad || currentScreen == GameScreen::Reading)) {
		gameState.gameOver = true;
		flash = 0.6f;
		flashColour = glm::vec3(0.8f, 0.0f, 0.0f);
		setScreen(GameScreen::GameOver);
	}

	if (currentScreen == GameScreen::Victory)
		exitFade = (glm::min)(0.85f, exitFade + dt * 0.9f);
	else if (currentScreen == GameScreen::GameOver)
		exitFade = (glm::min)(0.6f, exitFade + dt * 0.8f);
	else
		exitFade = 0.0f;
}

void Game::updateCursor() {
	bool capture = currentScreen == GameScreen::Playing;
	ImGuiIO& io = ImGui::GetIO();

	if (capture != cursorCaptured) {
		GLFWwindow* window = glfwGetCurrentContext();
		glfwSetInputMode(window, GLFW_CURSOR, capture ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
		cursorCaptured = capture;
		lookSuppressFrames = 3;		// ignore the jump when the cursor mode changes
	}

	// while looking around, ImGui must not react to the (hidden) mouse
	if (capture)
		io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
	else
		io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
}

void Game::renderWorld() {
	if (Scene::SCR_WIDTH == 0 || Scene::SCR_HEIGHT == 0)
		return;		// minimised

	FrameParams frame;
	frame.time = clock;
	frame.shadows = gameSettings.shadows;
	frame.torchOn = gameSettings.torchOn;
	frame.showColliders = gameSettings.showColliders;
	frame.fade = (glm::max)(exitFade, currentScreen == GameScreen::Paused ? 0.35f : 0.0f);
	frame.flash = flash;
	frame.flashColor = flashColour;

	float aspect = (float)Scene::SCR_WIDTH / (float)Scene::SCR_HEIGHT;
	frame.projection = glm::perspective(glm::radians(gameSettings.fov), aspect, 0.05f, 60.0f);

	if (currentScreen == GameScreen::MainMenu) {
		// slow look around the room behind the menu
		glm::vec3 eye(-0.5f, 1.75f, 4.2f);
		float yaw = glm::radians(-95.0f + 28.0f * std::sin(clock * 0.12f));
		float pitch = glm::radians(-9.0f + 3.0f * std::sin(clock * 0.21f));
		glm::vec3 front(std::cos(yaw) * std::cos(pitch), std::sin(pitch), std::sin(yaw) * std::cos(pitch));
		frame.eye = eye;
		frame.forward = front;
		frame.view = glm::lookAt(eye, eye + front, glm::vec3(0.0f, 1.0f, 0.0f));
		frame.torchOn = false;
	}
	else {
		frame.eye = thePlayer.eyePosition();
		frame.forward = thePlayer.forward();
		frame.view = thePlayer.viewMatrix();
	}

	if (currentScreen == GameScreen::Playing && interaction.target())
		frame.highlightGroup = interaction.target()->group();

	std::vector<Collision::AABB> debugBoxes;
	if (frame.showColliders) {
		GameContext ctx = context();
		for (auto& obj : world.interactables)
			if (obj->canTarget(ctx))
				debugBoxes.push_back(obj->bounds(ctx));
		frame.debugBoxes = &debugBoxes;
	}

	renderer.render(scene, world, frame);
}

std::string Game::objective() const {
	const GameState& s = gameState;
	if (s.gameCompleted)        return "You escaped.";
	if (exitDoorOpen())         return "Get out through the exit corridor.";
	if (s.exitUnlocked)         return "Open the exit door.";
	if (s.knowsExitCode)        return "Enter the exit code on the keypad by the door.";
	if (s.boxOpened)            return "Look inside the lockbox.";
	if (s.hasKey)               return "Find what the key opens.";
	if (s.cabinetUnlocked)      return "Take what's inside the cabinet.";
	if (s.hasReadNote)          return "Work out the cabinet combination.";
	return "Search the room for clues.";
}
