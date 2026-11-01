#ifndef ESCAPE_GAMEUI_H
#define ESCAPE_GAMEUI_H

#include <imgui.h>

class Game;

/*
	All of the game's UI, drawn with Dear ImGui (the engine's front end):
	main menu, HUD (crosshair, prompt, objective, inventory, timer, messages),
	keypad, note reader, pause menu, victory / game over screens and a debug panel.
*/
class GameUI {
public:
	void init();
	void draw(Game& game);

private:
	ImFont* bodyFont = nullptr;
	ImFont* smallFont = nullptr;
	ImFont* titleFont = nullptr;
	ImFont* digitFont = nullptr;	// keypad display

	void applyStyle();

	void drawMainMenu(Game& game);
	void drawHUD(Game& game);
	void drawKeypad(Game& game);
	void drawReadable(Game& game);
	void drawPause(Game& game);
	void drawVictory(Game& game);
	void drawGameOver(Game& game);
	void drawDebug(Game& game);
	void drawSettings(Game& game);

	bool bigButton(const char* label, float width);
};

#endif // !ESCAPE_GAMEUI_H
