#pragma once

#ifndef SCENE_H
#define SCENE_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <vector>

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "Shader.h"

#include "Keyboard.h"
#include "Mouse.h"
#include "Joystick.h"

#include "Camera.h"

class Scene {
public:
	static void framebufferSizeCallback(GLFWwindow* window, int width, int height);

	Scene();
	Scene(int glfwVersionMajor, int glfwVersionMinor, const char* title, unsigned int scrWidth, unsigned int scrHeight);

	bool init();
	void initFrontEnd();
	bool joystickPresent();
	void addCam(Camera* camera);
	void processInput(float dt);
	void update();
	void frontendNewFrame();
	void backendNewFrame();
	void render(Shader& shader, bool applyLighting = true);
	void renderIMGUI();
	void cleanup();
	bool shouldClose();
	Camera* getActiveCamera();
	void setShouldClose(bool shouldClose);
	void setWindowColor(float r, float g, float b, float a);

	/*
		Cameras
	*/
	std::vector<Camera*> cameras;
	unsigned int activeCam;
	glm::mat4 projection;
	glm::mat4 view;
	glm::vec3 cameraPos;

	static unsigned int SCR_WIDTH;
	static unsigned int SCR_HEIGHT;

	Joystick mainJ;

protected:
	GLFWwindow* window;
	const char* title;

	float bg[4];

	//GLFW info
	int glfwVersionMajor;
	int glfwVersionMinor;

};

#endif // !SCENE_H
