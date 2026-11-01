/*
	The Escape Room - a small first-person puzzle game built on the Graphics Engine.

	The engine provides the window, input, ImGui front end, camera, shaders, meshes,
	model loading, lights, shadow maps and post-processing. Everything game specific
	(rules, level, player, interaction, UI) lives in this project.
*/

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <memory>

#include "graphics/scene.h"
#include "graphics/Camera.h"
#include "memory/path.h"

#include "Game/Game.h"

// the engine declares this static in Camera.h and defines it in its own main.cpp
Camera Camera::defaultCamera(glm::vec3(0.0f, 0.0f, 0.0f));

int main() {
	try {
		Paths::init();
	}
	catch (const std::exception& e) {
		std::cout << e.what() << std::endl;
		return -1;
	}

	Scene scene(3, 3, "The Escape Room", 1280, 720);
	if (!scene.init()) {
		std::cout << "Failed to initialize GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	scene.initFrontEnd();		// ImGui context + GLFW / OpenGL3 back ends
	glfwSwapInterval(1);		// vsync: the engine leaves the frame rate uncapped

	// the game owns GL resources, so it must be destroyed before the context goes away
	std::unique_ptr<Game> game(new Game(scene));
	if (!game->init()) {
		std::cout << "Failed to initialise the game (missing shaders or assets next to the executable?)" << std::endl;
		game->cleanup();
		scene.cleanup();
		return -1;
	}

	float lastFrame = static_cast<float>(glfwGetTime());
	while (!scene.shouldClose()) {
		float now = static_cast<float>(glfwGetTime());
		float dt = now - lastFrame;
		lastFrame = now;
		if (dt > 0.1f)
			dt = 0.1f;		// e.g. after dragging the window: never take one huge step

		scene.frontendNewFrame();	// ImGui new frame
		game->frame(dt);			// input -> update -> render 3D -> build UI
		scene.renderIMGUI();		// draw the UI on top
		scene.backendNewFrame();	// swap buffers + poll events
	}

	game->cleanup();
	game.reset();
	scene.cleanup();
	return 0;
}
