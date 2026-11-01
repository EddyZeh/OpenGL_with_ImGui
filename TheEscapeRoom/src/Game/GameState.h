#ifndef ESCAPE_GAMESTATE_H
#define ESCAPE_GAMESTATE_H

#include <string>

/*
	Everything the game needs to remember about the player's progress.
	The rules that change these flags live in PuzzleManager and the interactable objects;
	rendering only ever reads them.
*/

// which UI screen currently owns the input
enum class GameScreen {
	MainMenu,
	Playing,
	Keypad,			// entering a code (cabinet or exit)
	Reading,		// a note / card is open
	Paused,
	Victory,
	GameOver
};

struct GameState {
	// progression
	bool hasReadNote = false;		// read the note on the desk
	bool hasInspectedBoard = false;	// looked at the clue board
	bool cabinetUnlocked = false;	// correct cabinet code entered
	bool hasKey = false;			// key collected from the cabinet
	bool boxOpened = false;			// lockbox on the table opened with the key
	bool knowsExitCode = false;		// read the card inside the lockbox
	bool exitUnlocked = false;		// correct exit code entered

	// outcome
	bool gameCompleted = false;		// walked through the exit
	bool gameOver = false;			// ran out of time (timed mode only)

	// stats / timer
	float elapsed = 0.0f;			// seconds of play (paused time excluded)
	bool timed = false;
	float timeLimit = 8.0f * 60.0f;
	int wrongCodes = 0;

	float timeRemaining() const {
		float t = timeLimit - elapsed;
		return t > 0.0f ? t : 0.0f;
	}
};

#endif // !ESCAPE_GAMESTATE_H
