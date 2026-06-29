#pragma once

#ifndef SCREEN_H
#define SCREEN_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "../io/Mouse.h"
#include "../io/Keyboard.h"
#include "../io/Joystick.h"

class Screen {
public:
	static unsigned int SCR_WIDTH;
	static unsigned int SCR_HEIGHT;

	GLFWwindow* window;

	static void framebufferSizeCallback(GLFWwindow* window, int width, int height);

	Screen();

	bool init();

	void setParamters();

	// main
	void update();
	void newFrame();

	// window closing accessor and modifier
	bool shouldClose();
	void setShouldClose(bool shouldClose);

};

#endif // !SCREEN_H

