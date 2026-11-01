#include "PuzzleManager.h"

#include <random>

void PuzzleManager::generate(unsigned int seed) {
	currentSeed = seed;
	std::mt19937 rng(seed);

	// the board's rows always appear in this order, top to bottom
	rows[0] = { "RED",    glm::vec3(1.00f, 0.15f, 0.10f), 0 };
	rows[1] = { "GREEN",  glm::vec3(0.15f, 1.00f, 0.25f), 0 };
	rows[2] = { "BLUE",   glm::vec3(0.20f, 0.45f, 1.00f), 0 };
	rows[3] = { "YELLOW", glm::vec3(1.00f, 0.85f, 0.10f), 0 };

	// 1..5 lights per row: few enough to count at a glance from across the room
	std::uniform_int_distribution<int> countDist(1, 5);
	cabinetDigits.clear();
	for (auto& row : rows) {
		row.count = countDist(rng);
		cabinetDigits += static_cast<char>('0' + row.count);
	}

	// exit code: any four digits, but never the same as the cabinet code
	std::uniform_int_distribution<int> digitDist(0, 9);
	do {
		exitDigits.clear();
		for (int i = 0; i < CODE_LENGTH; i++)
			exitDigits += static_cast<char>('0' + digitDist(rng));
	} while (exitDigits == cabinetDigits);
}

CodeResult PuzzleManager::submitCode(KeypadTarget target, const std::string& code, GameState& state) const {
	bool& unlocked = (target == KeypadTarget::Cabinet) ? state.cabinetUnlocked : state.exitUnlocked;
	const std::string& expected = (target == KeypadTarget::Cabinet) ? cabinetDigits : exitDigits;

	if (unlocked)
		return CodeResult::AlreadyUnlocked;

	if ((int)code.size() < CODE_LENGTH)
		return CodeResult::Incomplete;

	if (code == expected) {
		unlocked = true;
		return CodeResult::Accepted;
	}

	state.wrongCodes++;
	return CodeResult::Rejected;
}
