#ifndef ESCAPE_PUZZLEMANAGER_H
#define ESCAPE_PUZZLEMANAGER_H

#include <glm/glm.hpp>

#include <array>
#include <string>

#include "../Game/GameState.h"

/*
	Central place for the puzzle rules.

	  Clue board  ->  cabinet code (count the lights in each coloured row, top to bottom)
	  Cabinet     ->  key
	  Key         ->  lockbox on the table  ->  card with the exit code
	  Exit code   ->  exit door unlocks

	Every unlock goes through this class so the puzzle cannot be bypassed by an object
	deciding on its own that it should open.
*/

enum class KeypadTarget {
	Cabinet,
	Exit
};

enum class CodeResult {
	Accepted,
	Rejected,
	Incomplete,			// fewer digits than required
	AlreadyUnlocked
};

struct ClueRow {
	const char* colourName;
	glm::vec3 colour;
	int count;			// 1..5 lights in this row = one digit of the cabinet code
};

class PuzzleManager {
public:
	static constexpr int CODE_LENGTH = 4;

	// Build a fresh set of codes. Same seed -> same puzzle.
	void generate(unsigned int seed);

	const std::array<ClueRow, CODE_LENGTH>& clueRows() const { return rows; }
	const std::string& cabinetCode() const { return cabinetDigits; }
	const std::string& exitCode() const { return exitDigits; }

	// Validate a code typed on a keypad and update the game state when it is correct.
	CodeResult submitCode(KeypadTarget target, const std::string& code, GameState& state) const;

	// Unlock conditions, kept in one place
	bool canOpenLockBox(const GameState& s) const { return s.hasKey && !s.boxOpened; }
	bool canOpenExitDoor(const GameState& s) const { return s.exitUnlocked; }
	bool isKeyAvailable(const GameState& s) const { return s.cabinetUnlocked && !s.hasKey; }

	unsigned int seed() const { return currentSeed; }

private:
	std::array<ClueRow, CODE_LENGTH> rows{};
	std::string cabinetDigits;
	std::string exitDigits;
	unsigned int currentSeed = 0;
};

#endif // !ESCAPE_PUZZLEMANAGER_H
