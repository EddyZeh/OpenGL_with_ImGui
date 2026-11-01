#ifndef ESCAPE_KEYPAD_H
#define ESCAPE_KEYPAD_H

#include <string>

#include "../Interaction/Interactable.h"
#include "../Puzzles/PuzzleManager.h"

/*
	Keypad: the digits currently typed and the feedback shown on the keypad screen.
	It does not know any codes; PuzzleManager validates what is submitted.
	Used by both the cabinet and the exit (only the target differs).
*/
class Keypad {
public:
	enum class Feedback {
		None,
		Granted,
		Denied
	};

	void begin(KeypadTarget target);

	void pressDigit(int digit);
	void backspace();
	void clearEntry();
	CodeResult submit(const PuzzleManager& puzzles, GameState& state);

	void update(float dt);

	KeypadTarget target() const { return currentTarget; }
	const std::string& entered() const { return digits; }
	Feedback feedback() const { return feedbackKind; }
	const std::string& feedbackText() const { return text; }
	float feedbackTime() const { return feedbackTimer; }

	// becomes true shortly after a correct code so the player sees "ACCESS GRANTED"
	bool wantsClose() const { return closeRequested; }

private:
	KeypadTarget currentTarget = KeypadTarget::Cabinet;
	std::string digits;
	Feedback feedbackKind = Feedback::None;
	std::string text;
	float feedbackTimer = 0.0f;
	bool closeRequested = false;
};

/*
	The physical keypad next to the exit door: opens the keypad UI and shows
	red / green status depending on the game state.
*/
class ExitKeypadPanel : public Interactable {
public:
	ExitKeypadPanel(const Collision::AABB& box, int screenProp);

	std::string prompt(const GameContext& ctx) const override;
	void interact(GameContext& ctx) override;
	Collision::AABB bounds(const GameContext& ctx) const override;
	void update(float dt, GameContext& ctx) override;

private:
	Collision::AABB box;
	int screenProp;
	float blinkTimer = 0.0f;
};

#endif // !ESCAPE_KEYPAD_H
