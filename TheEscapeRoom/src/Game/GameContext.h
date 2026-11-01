#ifndef ESCAPE_GAMECONTEXT_H
#define ESCAPE_GAMECONTEXT_H

#include <string>
#include <vector>

#include "GameState.h"
#include "../Puzzles/PuzzleManager.h"

class Level;

/*
	Requests that gameplay objects make to the UI. Objects never open windows themselves;
	the Game reads these after the interaction step and switches screens.
*/
struct UIRequests {
	std::vector<std::string> messages;		// short HUD toasts

	bool openKeypad = false;
	KeypadTarget keypadTarget = KeypadTarget::Cabinet;

	bool openReadable = false;
	std::string readableTitle;
	std::string readableBody;

	void message(const std::string& text) { messages.push_back(text); }

	void keypad(KeypadTarget target) {
		openKeypad = true;
		keypadTarget = target;
	}

	void read(const std::string& title, const std::string& body) {
		openReadable = true;
		readableTitle = title;
		readableBody = body;
	}

	void clear() {
		messages.clear();
		openKeypad = false;
		openReadable = false;
	}
};

// What an interactable is allowed to see and change.
struct GameContext {
	GameState& state;
	const PuzzleManager& puzzles;
	Level& level;
	UIRequests& ui;
};

#endif // !ESCAPE_GAMECONTEXT_H
