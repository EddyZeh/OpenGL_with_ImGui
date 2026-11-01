#include "Keypad.h"

#include <cmath>

#include "../Game/GameContext.h"
#include "../Game/Level.h"

// ---------------------------------------------------------------------------- Keypad

void Keypad::begin(KeypadTarget target) {
	currentTarget = target;
	digits.clear();
	feedbackKind = Feedback::None;
	text.clear();
	feedbackTimer = 0.0f;
	closeRequested = false;
}

void Keypad::pressDigit(int digit) {
	if (feedbackKind == Feedback::Granted)
		return;		// locked in once accepted

	if (feedbackKind == Feedback::Denied) {
		// start a fresh attempt
		feedbackKind = Feedback::None;
		text.clear();
	}

	if (digit < 0 || digit > 9)
		return;
	if ((int)digits.size() >= PuzzleManager::CODE_LENGTH)
		return;

	digits += static_cast<char>('0' + digit);
}

void Keypad::backspace() {
	if (feedbackKind == Feedback::Granted)
		return;
	if (!digits.empty())
		digits.pop_back();
}

void Keypad::clearEntry() {
	if (feedbackKind == Feedback::Granted)
		return;
	digits.clear();
	feedbackKind = Feedback::None;
	text.clear();
}

CodeResult Keypad::submit(const PuzzleManager& puzzles, GameState& state) {
	if (feedbackKind == Feedback::Granted)
		return CodeResult::AlreadyUnlocked;

	CodeResult result = puzzles.submitCode(currentTarget, digits, state);
	feedbackTimer = 0.0f;

	switch (result) {
	case CodeResult::Accepted:
	case CodeResult::AlreadyUnlocked:
		feedbackKind = Feedback::Granted;
		text = "ACCESS GRANTED";
		break;

	case CodeResult::Rejected:
		feedbackKind = Feedback::Denied;
		text = "INCORRECT CODE";
		digits.clear();		// incorrect: clear the entry
		break;

	case CodeResult::Incomplete:
		feedbackKind = Feedback::Denied;
		text = "ENTER 4 DIGITS";
		break;
	}
	return result;
}

void Keypad::update(float dt) {
	if (feedbackKind == Feedback::None)
		return;

	feedbackTimer += dt;

	if (feedbackKind == Feedback::Granted && feedbackTimer > 1.0f)
		closeRequested = true;

	if (feedbackKind == Feedback::Denied && feedbackTimer > 1.6f) {
		feedbackKind = Feedback::None;
		text.clear();
	}
}

// ---------------------------------------------------------------------------- ExitKeypadPanel

ExitKeypadPanel::ExitKeypadPanel(const Collision::AABB& box, int screenProp)
	: Interactable("exit_keypad"), box(box.withMinThickness(0.15f)), screenProp(screenProp) {}

std::string ExitKeypadPanel::prompt(const GameContext& ctx) const {
	if (ctx.state.exitUnlocked)
		return "Keypad: ACCESS GRANTED";
	return "Press E to use the exit keypad";
}

void ExitKeypadPanel::interact(GameContext& ctx) {
	if (ctx.state.exitUnlocked) {
		ctx.ui.message("The door is unlocked.");
		return;
	}
	ctx.ui.keypad(KeypadTarget::Exit);
}

Collision::AABB ExitKeypadPanel::bounds(const GameContext& ctx) const {
	(void)ctx;
	return box;
}

void ExitKeypadPanel::update(float dt, GameContext& ctx) {
	if (screenProp < 0)
		return;

	Surface& s = ctx.level.props[screenProp].surface;
	if (ctx.state.exitUnlocked) {
		s = Surface::glow(glm::vec3(0.1f, 1.0f, 0.35f), 1.3f);
	}
	else {
		blinkTimer += dt;
		float pulse = 0.55f + 0.35f * std::sin(blinkTimer * 4.0f);
		s = Surface::glow(glm::vec3(1.0f, 0.12f, 0.08f), pulse);
	}
}
