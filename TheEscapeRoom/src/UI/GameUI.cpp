#include "GameUI.h"

#include <cfloat>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <string>

#include "memory/path.h"	// engine asset paths

#include "../Game/Game.h"

namespace {
	const ImVec4 AMBER(1.00f, 0.62f, 0.12f, 1.0f);
	const ImVec4 AMBER_DIM(0.70f, 0.42f, 0.08f, 1.0f);
	const ImVec4 PANEL(0.05f, 0.055f, 0.06f, 0.92f);
	const ImU32 WHITE = IM_COL32(235, 232, 225, 255);
	const ImU32 GREY = IM_COL32(160, 160, 160, 255);
	const ImU32 AMBER32 = IM_COL32(255, 160, 30, 255);
	const ImU32 GREEN32 = IM_COL32(60, 235, 110, 255);
	const ImU32 RED32 = IM_COL32(255, 70, 50, 255);

	std::string formatTime(float seconds) {
		int s = (int)std::ceil(seconds);
		if (s < 0) s = 0;
		char buf[16];
		std::snprintf(buf, sizeof(buf), "%02d:%02d", s / 60, s % 60);
		return buf;
	}

	ImVec2 textSize(ImFont* font, float size, const char* text) {
		return font->CalcTextSizeA(size, FLT_MAX, 0.0f, text);
	}

	void shadowText(ImDrawList* dl, ImFont* font, float size, ImVec2 pos, ImU32 col, const char* text) {
		ImU32 shadow = IM_COL32(0, 0, 0, (int)(((col >> IM_COL32_A_SHIFT) & 0xFF) * 0.75f));
		dl->AddText(font, size, ImVec2(pos.x + 1.5f, pos.y + 1.5f), shadow, text);
		dl->AddText(font, size, pos, col, text);
	}

	ImU32 withAlpha(ImU32 col, float a) {
		int alpha = (int)(((col >> IM_COL32_A_SHIFT) & 0xFF) * a);
		return (col & ~IM_COL32_A_MASK) | ((ImU32)alpha << IM_COL32_A_SHIFT);
	}

	void centreNextWindow(ImVec2 size) {
		ImGuiIO& io = ImGui::GetIO();
		ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		ImGui::SetNextWindowSize(size, ImGuiCond_Always);
	}

	const ImGuiWindowFlags MODAL_FLAGS = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse;
}

// =========================================================================== setup

void GameUI::init() {
	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = nullptr;		// the game has no dockable windows to remember
	io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

	std::filesystem::path fonts = Paths::Assets() / "fonts";
	std::string body = (fonts / "Roboto-Medium.ttf").string();
	std::string mono = (fonts / "Cousine-Regular.ttf").string();

	if (std::filesystem::exists(body)) {
		bodyFont = io.Fonts->AddFontFromFileTTF(body.c_str(), 19.0f);
		smallFont = io.Fonts->AddFontFromFileTTF(body.c_str(), 15.0f);
		titleFont = io.Fonts->AddFontFromFileTTF(body.c_str(), 46.0f);
	}
	if (std::filesystem::exists(mono))
		digitFont = io.Fonts->AddFontFromFileTTF(mono.c_str(), 44.0f);

	// fall back to ImGui's built-in font when the TTFs are missing
	if (!bodyFont) bodyFont = io.Fonts->AddFontDefault();
	if (!smallFont) smallFont = bodyFont;
	if (!titleFont) titleFont = bodyFont;
	if (!digitFont) digitFont = titleFont;

	applyStyle();
}

void GameUI::applyStyle() {
	ImGuiStyle& style = ImGui::GetStyle();
	ImGui::StyleColorsDark(&style);
	style.WindowRounding = 6.0f;
	style.FrameRounding = 4.0f;
	style.GrabRounding = 4.0f;
	style.WindowPadding = ImVec2(22.0f, 20.0f);
	style.FramePadding = ImVec2(10.0f, 7.0f);
	style.ItemSpacing = ImVec2(10.0f, 10.0f);
	style.WindowBorderSize = 1.0f;

	ImVec4* c = style.Colors;
	c[ImGuiCol_WindowBg] = PANEL;
	c[ImGuiCol_Border] = ImVec4(1.0f, 0.62f, 0.12f, 0.35f);
	c[ImGuiCol_Text] = ImVec4(0.93f, 0.92f, 0.89f, 1.0f);
	c[ImGuiCol_Button] = ImVec4(0.14f, 0.15f, 0.16f, 1.0f);
	c[ImGuiCol_ButtonHovered] = AMBER_DIM;
	c[ImGuiCol_ButtonActive] = AMBER;
	c[ImGuiCol_FrameBg] = ImVec4(0.14f, 0.15f, 0.16f, 1.0f);
	c[ImGuiCol_FrameBgHovered] = ImVec4(0.22f, 0.20f, 0.16f, 1.0f);
	c[ImGuiCol_FrameBgActive] = ImVec4(0.30f, 0.24f, 0.14f, 1.0f);
	c[ImGuiCol_CheckMark] = AMBER;
	c[ImGuiCol_SliderGrab] = AMBER_DIM;
	c[ImGuiCol_SliderGrabActive] = AMBER;
	c[ImGuiCol_Header] = ImVec4(0.18f, 0.16f, 0.12f, 1.0f);
	c[ImGuiCol_HeaderHovered] = AMBER_DIM;
	c[ImGuiCol_HeaderActive] = AMBER;
	c[ImGuiCol_Separator] = ImVec4(1.0f, 0.62f, 0.12f, 0.3f);
}

bool GameUI::bigButton(const char* label, float width) {
	return ImGui::Button(label, ImVec2(width, 44.0f));
}

// =========================================================================== dispatch

void GameUI::draw(Game& game) {
	ImGui::PushFont(bodyFont);

	switch (game.screen()) {
	case GameScreen::MainMenu:
		drawMainMenu(game);
		break;
	case GameScreen::Playing:
		drawHUD(game);
		break;
	case GameScreen::Keypad:
		drawHUD(game);
		drawKeypad(game);
		break;
	case GameScreen::Reading:
		drawHUD(game);
		drawReadable(game);
		break;
	case GameScreen::Paused:
		drawPause(game);
		break;
	case GameScreen::Victory:
		drawVictory(game);
		break;
	case GameScreen::GameOver:
		drawGameOver(game);
		break;
	}

	if (game.settings().showDebug)
		drawDebug(game);

	ImGui::PopFont();
}

// =========================================================================== main menu

void GameUI::drawMainMenu(Game& game) {
	ImGuiIO& io = ImGui::GetIO();
	ImDrawList* bg = ImGui::GetBackgroundDrawList();

	// darken the left side so the text reads over the room
	bg->AddRectFilledMultiColor(ImVec2(0, 0), ImVec2(io.DisplaySize.x * 0.65f, io.DisplaySize.y),
		IM_COL32(0, 0, 0, 225), IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 225));

	const float width = 470.0f;
	ImGui::SetNextWindowPos(ImVec2(70.0f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.0f, 0.5f));
	ImGui::SetNextWindowSize(ImVec2(width, 0.0f), ImGuiCond_Always);
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
	ImGui::Begin("##mainmenu", nullptr, MODAL_FLAGS | ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::PushFont(titleFont);
	ImGui::TextColored(AMBER, "THE ESCAPE ROOM");
	ImGui::PopFont();
	ImGui::TextDisabled("PROJECT LOCKDOWN");
	ImGui::Spacing();
	ImGui::PushTextWrapPos(width - 30.0f);
	ImGui::TextUnformatted("You wake up inside a small, abandoned underground facility. "
		"The main exit is sealed and the security system wants a code. "
		"Everything you need is somewhere in this room.");
	ImGui::PopTextWrapPos();
	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	const float bw = width - 44.0f;
	if (bigButton("START", bw))
		game.startNewGame();

	GameSettings& s = game.settings();
	ImGui::Checkbox("Timed mode", &s.timed);
	if (s.timed) {
		ImGui::SameLine();
		ImGui::SetNextItemWidth(170.0f);
		ImGui::SliderInt("##limit", &s.timeLimitMinutes, 3, 15, "%d minutes");
	}

	if (ImGui::CollapsingHeader("Controls")) {
		ImGui::PushFont(smallFont);
		ImGui::BulletText("WASD / arrows   move        Shift   sprint");
		ImGui::BulletText("Mouse           look        E       interact");
		ImGui::BulletText("F               torch       Esc     pause / close");
		ImGui::BulletText("0-9, Enter, Backspace on the keypad");
		ImGui::BulletText("F1 debug panel   F2 show collision boxes");
		ImGui::BulletText("Gamepad: sticks move/look, A interact, Start pause");
		ImGui::PopFont();
	}
	if (ImGui::CollapsingHeader("Settings"))
		drawSettings(game);

	ImGui::Spacing();
	if (bigButton("QUIT", bw))
		game.quitGame();

	ImGui::End();
	ImGui::PopStyleColor(2);
}

void GameUI::drawSettings(Game& game) {
	GameSettings& s = game.settings();
	ImGui::PushFont(smallFont);
	ImGui::SetNextItemWidth(220.0f);
	ImGui::SliderFloat("Mouse sensitivity", &s.mouseSensitivity, 0.02f, 0.4f, "%.2f");
	ImGui::SetNextItemWidth(220.0f);
	ImGui::SliderFloat("Field of view", &s.fov, 45.0f, 90.0f, "%.0f deg");
	ImGui::Checkbox("Invert Y", &s.invertY);
	ImGui::SameLine();
	ImGui::Checkbox("Head bob", &s.headBob);
	ImGui::SameLine();
	ImGui::Checkbox("Shadows", &s.shadows);
	ImGui::PopFont();
}

// =========================================================================== HUD

void GameUI::drawHUD(Game& game) {
	ImGuiIO& io = ImGui::GetIO();
	ImDrawList* dl = ImGui::GetBackgroundDrawList();
	const ImVec2 size = io.DisplaySize;
	const ImVec2 centre(size.x * 0.5f, size.y * 0.5f);
	const GameState& st = game.state();
	const bool playing = game.screen() == GameScreen::Playing;

	// ---- crosshair --------------------------------------------------------------------
	if (playing) {
		if (game.prompt().empty()) {
			dl->AddCircleFilled(centre, 2.5f, IM_COL32(255, 255, 255, 200));
		}
		else {
			dl->AddCircle(centre, 9.0f, AMBER32, 24, 2.0f);
			dl->AddCircleFilled(centre, 2.5f, AMBER32);
		}
	}

	// ---- interaction prompt -------------------------------------------------------------
	if (playing && !game.prompt().empty()) {
		const char* text = game.prompt().c_str();
		ImVec2 ts = textSize(bodyFont, 21.0f, text);
		ImVec2 pos(centre.x - ts.x * 0.5f, size.y * 0.66f);
		dl->AddRectFilled(ImVec2(pos.x - 16, pos.y - 9), ImVec2(pos.x + ts.x + 16, pos.y + ts.y + 9), IM_COL32(0, 0, 0, 160), 6.0f);
		dl->AddRect(ImVec2(pos.x - 16, pos.y - 9), ImVec2(pos.x + ts.x + 16, pos.y + ts.y + 9), IM_COL32(255, 160, 30, 120), 6.0f);
		shadowText(dl, bodyFont, 21.0f, pos, WHITE, text);
	}

	// ---- objective (top left) -------------------------------------------------------------
	shadowText(dl, smallFont, 15.0f, ImVec2(24, 20), AMBER32, "OBJECTIVE");
	shadowText(dl, bodyFont, 20.0f, ImVec2(24, 40), WHITE, game.objective().c_str());

	// ---- timer (top right) -----------------------------------------------------------------
	{
		std::string t;
		ImU32 col = GREY;
		const char* label = "TIME";
		if (st.timed) {
			float left = st.timeRemaining();
			t = formatTime(left);
			label = "LOCKDOWN IN";
			col = left < 60.0f ? (std::fmod(left, 1.0f) > 0.5f ? RED32 : IM_COL32(255, 150, 140, 255)) : WHITE;
		}
		else {
			t = formatTime(st.elapsed);
		}
		ImVec2 ls = textSize(smallFont, 15.0f, label);
		ImVec2 ts = textSize(titleFont, st.timed ? 38.0f : 26.0f, t.c_str());
		shadowText(dl, smallFont, 15.0f, ImVec2(size.x - 24 - ls.x, 20), st.timed ? AMBER32 : GREY, label);
		shadowText(dl, titleFont, st.timed ? 38.0f : 26.0f, ImVec2(size.x - 24 - ts.x, 38), col, t.c_str());
	}

	// ---- inventory (bottom left) -------------------------------------------------------------
	{
		float x = 24.0f, y = size.y - 128.0f;
		dl->AddRectFilled(ImVec2(x - 10, y - 10), ImVec2(x + 250, y + 104), IM_COL32(0, 0, 0, 120), 6.0f);
		shadowText(dl, smallFont, 15.0f, ImVec2(x, y), AMBER32, "INVENTORY");

		std::string key = st.hasKey ? (st.boxOpened ? "Key: Used" : "Key: Collected") : "Key: Not found";
		shadowText(dl, bodyFont, 19.0f, ImVec2(x, y + 22), st.hasKey ? GREEN32 : WHITE, key.c_str());

		std::string note = st.hasReadNote ? "Note: Read" : "Note: -";
		shadowText(dl, bodyFont, 19.0f, ImVec2(x, y + 46), st.hasReadNote ? WHITE : GREY, note.c_str());

		std::string code = st.knowsExitCode ? "Exit code: " + game.puzzles().exitCode() : "Exit code: ????";
		if (st.exitUnlocked) code = "Exit: UNLOCKED";
		shadowText(dl, bodyFont, 19.0f, ImVec2(x, y + 70), st.exitUnlocked ? GREEN32 : (st.knowsExitCode ? AMBER32 : GREY), code.c_str());
	}

	// ---- messages (stacked above the prompt) ---------------------------------------------------
	{
		float y = size.y - 64.0f;	// bottom centre, newest at the bottom
		const auto& toasts = game.toasts();
		for (auto it = toasts.rbegin(); it != toasts.rend(); ++it) {
			float a = 1.0f;
			if (it->age < 0.25f) a = it->age / 0.25f;
			if (it->age > 4.0f) a = 1.0f - (it->age - 4.0f);
			if (a <= 0.0f) continue;

			ImVec2 ts = textSize(bodyFont, 19.0f, it->text.c_str());
			ImVec2 pos(centre.x - ts.x * 0.5f, y);
			dl->AddRectFilled(ImVec2(pos.x - 12, pos.y - 5), ImVec2(pos.x + ts.x + 12, pos.y + ts.y + 5), IM_COL32(0, 0, 0, (int)(130 * a)), 4.0f);
			shadowText(dl, bodyFont, 19.0f, pos, withAlpha(WHITE, a), it->text.c_str());
			y -= ts.y + 16.0f;
		}
	}

	// torch hint
	if (!game.settings().torchOn && playing)
		shadowText(dl, smallFont, 15.0f, ImVec2(24, 70), GREY, "Torch off (F)");
}

// =========================================================================== keypad

void GameUI::drawKeypad(Game& game) {
	const Keypad& kp = game.keypad();

	// keyboard entry
	for (int d = 0; d <= 9; d++) {
		if (ImGui::IsKeyPressed((ImGuiKey)(ImGuiKey_0 + d), false) ||
			ImGui::IsKeyPressed((ImGuiKey)(ImGuiKey_Keypad0 + d), false))
			game.keypadDigit(d);
	}
	if (ImGui::IsKeyPressed(ImGuiKey_Backspace, true))
		game.keypadBackspace();
	if (ImGui::IsKeyPressed(ImGuiKey_Delete, false))
		game.keypadClear();
	if (ImGui::IsKeyPressed(ImGuiKey_Enter, false) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter, false))
		game.keypadSubmit();

	// shake on a wrong code
	float shake = 0.0f;
	if (kp.feedback() == Keypad::Feedback::Denied && kp.feedbackTime() < 0.4f)
		shake = std::sin(kp.feedbackTime() * 70.0f) * 10.0f * (1.0f - kp.feedbackTime() / 0.4f);

	ImGuiIO& io = ImGui::GetIO();
	const ImVec2 winSize(330.0f, 560.0f);
	ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f + shake, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(winSize, ImGuiCond_Always);
	ImGui::Begin("##keypad", nullptr, MODAL_FLAGS);

	const bool cabinet = kp.target() == KeypadTarget::Cabinet;
	ImGui::PushFont(smallFont);
	ImGui::TextColored(AMBER, "SECURITY");
	ImGui::SameLine(ImGui::GetContentRegionAvail().x - 70.0f);
	ImGui::TextDisabled(cabinet ? "CABINET" : "EXIT B3");
	ImGui::PopFont();

	// display: four digit slots
	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec2 p = ImGui::GetCursorScreenPos();
	float w = ImGui::GetContentRegionAvail().x;
	ImU32 screenCol = IM_COL32(8, 20, 12, 255);
	ImU32 digitCol = GREEN32;
	if (kp.feedback() == Keypad::Feedback::Denied) { screenCol = IM_COL32(30, 6, 6, 255); digitCol = RED32; }
	if (kp.feedback() == Keypad::Feedback::Granted) { screenCol = IM_COL32(6, 34, 14, 255); }
	dl->AddRectFilled(p, ImVec2(p.x + w, p.y + 92), screenCol, 6.0f);
	dl->AddRect(p, ImVec2(p.x + w, p.y + 92), IM_COL32(255, 255, 255, 30), 6.0f);

	const std::string& entered = kp.entered();
	float slotW = 50.0f, gap = 14.0f;
	float startX = p.x + (w - (slotW * 4 + gap * 3)) * 0.5f;
	for (int i = 0; i < PuzzleManager::CODE_LENGTH; i++) {
		float x = startX + i * (slotW + gap);
		char ch[2] = { i < (int)entered.size() ? entered[i] : '_', 0 };
		bool blink = i == (int)entered.size() && std::fmod(game.screenTime(), 1.0f) < 0.5f && kp.feedback() == Keypad::Feedback::None;
		ImVec2 cs = textSize(digitFont, 44.0f, ch);
		ImU32 col = (i < (int)entered.size()) ? digitCol : withAlpha(digitCol, blink ? 0.9f : 0.35f);
		dl->AddText(digitFont, 44.0f, ImVec2(x + (slotW - cs.x) * 0.5f, p.y + 10), col, ch);
	}

	const char* status = kp.feedbackText().empty() ? "ENTER CODE" : kp.feedbackText().c_str();
	ImVec2 ss = textSize(smallFont, 15.0f, status);
	dl->AddText(smallFont, 15.0f, ImVec2(p.x + (w - ss.x) * 0.5f, p.y + 64), withAlpha(digitCol, 0.85f), status);
	ImGui::Dummy(ImVec2(w, 100.0f));

	// buttons
	const float bw = (w - 2 * 10.0f) / 3.0f, bh = 58.0f;
	ImGui::PushFont(titleFont);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
	const int layout[3][3] = { { 1, 2, 3 }, { 4, 5, 6 }, { 7, 8, 9 } };
	for (int r = 0; r < 3; r++) {
		for (int c = 0; c < 3; c++) {
			if (c > 0) ImGui::SameLine();
			char label[8];
			std::snprintf(label, sizeof(label), "%d", layout[r][c]);
			if (ImGui::Button(label, ImVec2(bw, bh)))
				game.keypadDigit(layout[r][c]);
		}
	}
	ImGui::PopFont();

	ImGui::PushFont(bodyFont);
	if (ImGui::Button("CLR", ImVec2(bw, bh))) game.keypadClear();
	ImGui::SameLine();
	ImGui::PushFont(titleFont);
	if (ImGui::Button("0", ImVec2(bw, bh))) game.keypadDigit(0);
	ImGui::PopFont();
	ImGui::SameLine();
	if (ImGui::Button("DEL", ImVec2(bw, bh))) game.keypadBackspace();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.10f, 0.32f, 0.16f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.14f, 0.50f, 0.24f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.20f, 0.70f, 0.32f, 1.0f));
	if (ImGui::Button("ENTER", ImVec2(w, 50.0f))) game.keypadSubmit();
	ImGui::PopStyleColor(3);
	ImGui::PopFont();
	ImGui::PopStyleVar();

	ImGui::PushFont(smallFont);
	if (ImGui::Button("Step away  (Esc)", ImVec2(w, 0.0f)))
		game.closeKeypad();
	ImGui::PopFont();

	ImGui::End();
}

// =========================================================================== readable

void GameUI::drawReadable(Game& game) {
	ImGuiIO& io = ImGui::GetIO();
	ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(0, 0), io.DisplaySize, IM_COL32(0, 0, 0, 110));

	centreNextWindow(ImVec2(560.0f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.90f, 0.87f, 0.78f, 0.98f));
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.12f, 0.10f, 0.08f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.5f, 0.45f, 0.35f, 1.0f));
	ImGui::Begin("##readable", nullptr, MODAL_FLAGS | ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::PushFont(smallFont);
	ImGui::TextColored(ImVec4(0.45f, 0.30f, 0.10f, 1.0f), "%s", game.readableTitle().c_str());
	ImGui::PopFont();
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + 510.0f);
	ImGui::TextUnformatted(game.readableBody().c_str());
	ImGui::PopTextWrapPos();

	ImGui::Spacing();
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.93f, 0.92f, 0.89f, 1.0f));
	if (ImGui::Button("Put it down  (E / Esc)", ImVec2(-FLT_MIN, 0.0f)))
		game.closeReadable();
	ImGui::PopStyleColor();

	ImGui::End();
	ImGui::PopStyleColor(3);
}

// =========================================================================== pause / end screens

void GameUI::drawPause(Game& game) {
	centreNextWindow(ImVec2(380.0f, 0.0f));
	ImGui::Begin("##pause", nullptr, MODAL_FLAGS | ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::PushFont(titleFont);
	ImGui::TextColored(AMBER, "PAUSED");
	ImGui::PopFont();
	ImGui::TextDisabled("Objective: %s", game.objective().c_str());
	ImGui::Spacing();

	const float bw = 336.0f;
	if (bigButton("RESUME", bw)) game.resume();
	if (bigButton("RESTART", bw)) game.startNewGame();
	if (ImGui::CollapsingHeader("Settings"))
		drawSettings(game);
	if (bigButton("MAIN MENU", bw)) game.quitToMenu();
	if (bigButton("QUIT GAME", bw)) game.quitGame();

	ImGui::End();
}

void GameUI::drawVictory(Game& game) {
	if (game.screenTime() < 0.9f)
		return;		// let the fade play first

	const GameState& st = game.state();
	centreNextWindow(ImVec2(440.0f, 0.0f));
	ImGui::SetNextWindowBgAlpha((glm::min)(0.92f, (game.screenTime() - 0.9f) * 2.0f));
	ImGui::Begin("##victory", nullptr, MODAL_FLAGS | ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::PushFont(titleFont);
	ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.5f, 1.0f), "YOU ESCAPED");
	ImGui::PopFont();
	ImGui::TextWrapped("Daylight. Whatever happened down there, you're out.");
	ImGui::Spacing();
	ImGui::Text("Time:            %s", formatTime(st.elapsed).c_str());
	ImGui::Text("Wrong codes:     %d", st.wrongCodes);
	if (st.timed)
		ImGui::Text("Time to spare:   %s", formatTime(st.timeRemaining()).c_str());
	ImGui::Spacing();

	const float bw = 396.0f;
	if (bigButton("PLAY AGAIN (new codes)", bw)) game.startNewGame();
	if (bigButton("MAIN MENU", bw)) game.quitToMenu();
	if (bigButton("QUIT", bw)) game.quitGame();
	ImGui::End();
}

void GameUI::drawGameOver(Game& game) {
	if (game.screenTime() < 0.6f)
		return;

	centreNextWindow(ImVec2(440.0f, 0.0f));
	ImGui::Begin("##gameover", nullptr, MODAL_FLAGS | ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::PushFont(titleFont);
	ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.2f, 1.0f), "LOCKDOWN");
	ImGui::PopFont();
	ImGui::TextWrapped("The facility seals itself for good. You ran out of time.");
	ImGui::Spacing();

	const float bw = 396.0f;
	if (bigButton("TRY AGAIN", bw)) game.startNewGame();
	if (bigButton("MAIN MENU", bw)) game.quitToMenu();
	if (bigButton("QUIT", bw)) game.quitGame();
	ImGui::End();
}

// =========================================================================== debug (F1)

void GameUI::drawDebug(Game& game) {
	ImGuiIO& io = ImGui::GetIO();
	ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 20.0f, 110.0f), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
	ImGui::SetNextWindowBgAlpha(0.8f);
	ImGui::PushFont(smallFont);
	ImGui::Begin("Debug (F1)", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoFocusOnAppearing);

	const GameState& st = game.state();
	const Player& p = game.player();
	ImGui::Text("%.0f fps   %d draws", game.fps(), game.drawCalls());
	ImGui::Text("feet  %.2f %.2f %.2f", p.feet.x, p.feet.y, p.feet.z);
	ImGui::Text("props %d   colliders %d", (int)game.level().props.size(), (int)game.level().colliders.size());
	ImGui::Separator();
	ImGui::Text("note %d  board %d  cabinet %d  key %d", st.hasReadNote, st.hasInspectedBoard, st.cabinetUnlocked, st.hasKey);
	ImGui::Text("box %d  card %d  exit %d  door open %d", st.boxOpened, st.knowsExitCode, st.exitUnlocked, game.exitDoorOpen());
	ImGui::Text("codes: cabinet %s  exit %s", game.puzzles().cabinetCode().c_str(), game.puzzles().exitCode().c_str());
	ImGui::Separator();
	GameSettings& s = game.settings();
	ImGui::Checkbox("Collision boxes (F2)", &s.showColliders);
	ImGui::Checkbox("Shadows", &s.shadows);
	ImGui::Checkbox("Torch (F)", &s.torchOn);

	ImGui::End();
	ImGui::PopFont();
}
