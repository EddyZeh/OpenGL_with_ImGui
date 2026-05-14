#pragma once

#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLFW_GAMEPAD_BUTTON_A               0  // A (Xbox) / Cross (PS)
#define GLFW_GAMEPAD_BUTTON_B               1  // B (Xbox) / Circle (PS)
#define GLFW_GAMEPAD_BUTTON_X               2  // X (Xbox) / Square (PS)
#define GLFW_GAMEPAD_BUTTON_Y               3  // Y (Xbox) / Triangle (PS)
#define GLFW_GAMEPAD_BUTTON_LEFT_BUMPER     4  // LB / L1
#define GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER    5  // RB / R1
#define GLFW_GAMEPAD_BUTTON_BACK            6  // View / Share
#define GLFW_GAMEPAD_BUTTON_START           7  // Menu / Options
#define GLFW_GAMEPAD_BUTTON_GUIDE           8  // Xbox Button / PS Button
#define GLFW_GAMEPAD_BUTTON_LEFT_THUMB      9  // L3
#define GLFW_GAMEPAD_BUTTON_RIGHT_THUMB    10  // R3
#define GLFW_GAMEPAD_BUTTON_DPAD_UP        11
#define GLFW_GAMEPAD_BUTTON_DPAD_RIGHT     12
#define GLFW_GAMEPAD_BUTTON_DPAD_DOWN      13
#define GLFW_GAMEPAD_BUTTON_DPAD_LEFT      14
#define GLFW_GAMEPAD_BUTTON_LAST           GLFW_GAMEPAD_BUTTON_DPAD_LEFT

#define GLFW_GAMEPAD_AXIS_LEFT_X            0
#define GLFW_GAMEPAD_AXIS_LEFT_Y            1
#define GLFW_GAMEPAD_AXIS_RIGHT_X           2
#define GLFW_GAMEPAD_AXIS_RIGHT_Y           3
#define GLFW_GAMEPAD_AXIS_LEFT_TRIGGER      4
#define GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER     5
#define GLFW_GAMEPAD_AXIS_LAST              GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER

class Joystick {
public:
	Joystick(int i);

	void update();

	float axesState(int axis);
	unsigned char buttonState(int button);

	int getAxesCount();
	int getButtonCount();


	bool isPresent();
	const char* getName();

	static int getId(int i);


private:
	int present;

	int id;

	const char* name;

	int axesCount;
	const float* axes;

	int buttonCount;
	const unsigned char* buttons;

};


#endif // !JOYSTICK_H

