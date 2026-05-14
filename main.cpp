#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

#include "Screen.h"

#include "Shader.h"
#include "Texture.h"
#include "Camera.h"
#include "glMemory.hpp"

#include "Mouse.h"
#include "Keyboard.h"
#include "Joystick.h"

#include "cube.hpp"
#include "lamp.hpp"

void processInput(float deltaTime);
void renderQuad();


const unsigned int SCR_WIDTH  = 800;
const unsigned int SCR_HEIGHT = 600;

Screen screen;

Camera Camera::defaultCamera(glm::vec3(0.0f, 0.0f, 0.0f));
Joystick mainJ(0);
ArrayObject Cube::VAO;
bool Cube::initialized = false;

static bool quadInit = false;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

bool blinn = false, shadows = true;

int main() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	if (!screen.init()) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	screen.setParamters();

	// Configure global opengl state
	glEnable(GL_DEPTH_TEST);

	// SHADERS
	//Shader cubeShader("cube.vert", "cube.frag");
	Shader lampShader("lamp.vert", "lamp.frag");
	//Shader woodShader("wood.vert", "wood.frag");
	//Shader depthMapShader("depthMap.vert", "depthMap.frag");
	//Shader debugDepthQuad("debugDepthQuad.vert", "debugDepthQuad.frag");
	Shader shadowShader("shadows.vert", "shadows.frag");
	Shader cubemapShader("cubemap.vert", "cubemap.frag", "cubemap.geom");

	// TEXTURES
	Texture containerTex("assets", "container2.png");
	Texture containerSpec("assets", "container2_specular.png");
	Texture woodTex("assets", "wood.png");
	containerTex.load();
	containerSpec.load();
	woodTex.load();
	std::vector<Texture> textures = { containerTex };

	containerTex.bind();
	

	// MODELS
	Cube cubes[] = { Cube(glm::vec3(0.0f, 1.5f, 0.0f), glm::vec3(1.0f), textures),
					 Cube(glm::vec3(2.0f, 0.0f, 1.0f), glm::vec3(0.5f), textures),
					 Cube(glm::vec3(-1.0f, 0.0f, 2.0f), glm::vec3(0.25f), textures) 
					 };
	Cube roomCube(glm::vec3(0.0f, -0.5f, 0.0f), glm::vec3(6.0f), std::vector<Texture>());
	Lamp lamp(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.2f), Material::white_plastic);

	float woodVertices[] = {
		// positions            // normals         // texcoords
		 10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f,
		-10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
		-10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,   0.0f, 10.0f,

		 10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f,
		-10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,   0.0f, 10.0f,
		 10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,  10.0f, 10.0f
	};

	ArrayObject VAO;
	VAO.generate();
	VAO["VBO"] = BufferObject(GL_ARRAY_BUFFER);
	VAO.bind();
	VAO["VBO"].bind();
	VAO["VBO"].setData<float>(sizeof(woodVertices) / sizeof(woodVertices[0]), woodVertices, GL_STATIC_DRAW);
	VAO["VBO"].setAttPointer<float>(0, 3, GL_FLOAT, 8, 0);
	VAO["VB0"].setAttPointer<float>(1, 3, GL_FLOAT, 8, 3);
	VAO["VB0"].setAttPointer<float>(2, 2, GL_FLOAT, 8, 6);

	ArrayObject::clear();

	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);

	unsigned int depthCubemap;
	glGenTextures(1, &depthCubemap);

	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
	for (unsigned int i = 0; i < 6; i++) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, 
			GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	mainJ.update();
	if (mainJ.isPresent()) {
		std::cout << mainJ.getName() << " is present." << std::endl;
	}
	else {
		std::cout << "Not present." << std::endl;
	}

	shadowShader.activate();
	shadowShader.setInt("diffuseTexture", 0);
	shadowShader.setInt("depthMap", 1);

	// MAIN LOOP
	while (!screen.shouldClose()) {
		// INPUT
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		processInput(deltaTime);

		// RENDERING
		screen.update();

		// 1st PASS (Rendering DepthMap) ======================

		float aspect = (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT;
		float near_plane = 1.0f, far_plane = 25.0f;
		glm::mat4 shadowproj = glm::perspective(glm::radians(90.0f), aspect, near_plane, far_plane);

		std::vector<glm::mat4> shadowTransforms;
		shadowTransforms.push_back(shadowproj * 
							       glm::lookAt(lamp.pos, lamp.pos + glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)));
		shadowTransforms.push_back(shadowproj * 
							       glm::lookAt(lamp.pos, lamp.pos + glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)));
		shadowTransforms.push_back(shadowproj * 
							       glm::lookAt(lamp.pos, lamp.pos + glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)));
		shadowTransforms.push_back(shadowproj * 
							       glm::lookAt(lamp.pos, lamp.pos + glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)));
		shadowTransforms.push_back(shadowproj * 
							       glm::lookAt(lamp.pos, lamp.pos + glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)));
		shadowTransforms.push_back(shadowproj * 
							       glm::lookAt(lamp.pos, lamp.pos + glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f)));

		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
			glClear(GL_DEPTH_BUFFER_BIT);
			cubemapShader.activate();
			for (unsigned int i = 0; i < 6; ++i)
				cubemapShader.setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
			cubemapShader.setFloat("far_plane", far_plane);
			cubemapShader.set3Float("lightPos", lamp.pos);
			//cubemapShader.setMat4("model", glm::mat4(1.0f));
			//VAO.bind();
			//glDrawArrays(GL_TRIANGLES, 0, 6);
			roomCube.render(cubemapShader);
			for (auto& cube : cubes)
				cube.render(cubemapShader);
			lamp.render(cubemapShader);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
		// END ==================================
		
		// RESET VIEWPORT
		screen.update();

		// 2nd PASS (Rendering normal scene)
		
		glm::mat4 view = Camera::defaultCamera.getViewMatrix();
		glm::mat4 projection = glm::mat4(1.0f);
		projection = glm::perspective(glm::radians(Camera::defaultCamera.getZoom()), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.f);

		shadowShader.activate();
		shadowShader.setMat4("view", view);
		shadowShader.setMat4("projection", projection);

		shadowShader.set3Float("lightPos", lamp.pos);
		shadowShader.set3Float("viewPos", Camera::defaultCamera.cameraPos);
		shadowShader.setInt("shadows", shadows);
		shadowShader.setFloat("far_plane", far_plane);
		glActiveTexture(GL_TEXTURE0);
		woodTex.bind();
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
		shadowShader.setInt("reverse_normals", 1);
		roomCube.render(shadowShader);
		shadowShader.setInt("reverse_normals", 0);		
		//shadowShader.setMat4("model", glm::mat4(1.0f));
		//VAO.bind();
		//glDrawArrays(GL_TRIANGLES, 0, 6);
		for(auto &cube: cubes)
			cube.render(shadowShader);
		lampShader.activate();
		lampShader.setMat4("view", view);
		lampShader.setMat4("projection", projection);
		lamp.render(lampShader);
		

		ArrayObject::clear();

		// CHECK AND CALL EVENTS AND SWAP THE BUFFERS
		screen.newFrame();
	}
	for(auto &cube: cubes)
		cube.cleanup();
	lamp.cleanup();

	glfwTerminate();
	return 0;
}

void processInput(float deltaTime){
	// KEYBOARD =============================
	if (Keyboard::key(GLFW_KEY_ESCAPE)) {
		screen.setShouldClose(true);
	}
	if (Keyboard::key(GLFW_KEY_W)) {
		Camera::defaultCamera.updateCameraPos(CameraDirection::FORWARD, deltaTime);
	}

	if (Keyboard::key(GLFW_KEY_S)) {
		Camera::defaultCamera.updateCameraPos(CameraDirection::BACKWARD, deltaTime);
	}

	if (Keyboard::key(GLFW_KEY_D)) {
		Camera::defaultCamera.updateCameraPos(CameraDirection::RIGHT, deltaTime);
	}

	if (Keyboard::key(GLFW_KEY_A)) {
		Camera::defaultCamera.updateCameraPos(CameraDirection::LEFT, deltaTime);
	}

	if (Keyboard::key(GLFW_KEY_SPACE)) {
		Camera::defaultCamera.updateCameraPos(CameraDirection::UP, deltaTime);
	}

	if (Keyboard::key(GLFW_KEY_LEFT_SHIFT)) {
		Camera::defaultCamera.updateCameraPos(CameraDirection::DOWN, deltaTime);
	}

	if (Keyboard::keyWentDown(GLFW_KEY_B)) {
		blinn = !blinn;
	}
	if (Keyboard::keyWentDown(GLFW_KEY_Y)) {
		shadows = !shadows;
	}


	// MOUSE =============================
	double dx = Mouse::getDX(), dy = Mouse::getDY();
	if (dx != 0 || dy != 0) {
		Camera::defaultCamera.updateCameraDirection(dx, dy);
	}

	double scrollDY = Mouse::getScrollDY();
	if (scrollDY != 0) {
		Camera::defaultCamera.updateCameraZoom(scrollDY);
	}

	mainJ.update();

	// JOYSTIC =============================
	float lx = mainJ.axesState(GLFW_GAMEPAD_AXIS_LEFT_X);
	float ly = -mainJ.axesState(GLFW_GAMEPAD_AXIS_LEFT_Y);


	float sensitivity = 50.0f;
	float ry = -mainJ.axesState(GLFW_GAMEPAD_AXIS_RIGHT_Y);
	float rx = mainJ.axesState(GLFW_GAMEPAD_AXIS_RIGHT_X);

	if (lx > 0.5f) {
		Camera::defaultCamera.updateCameraPos(CameraDirection::RIGHT, deltaTime);
	}

	if (lx < -0.5f) {
		Camera::defaultCamera.updateCameraPos(CameraDirection::LEFT, deltaTime);
	}

	if (ly > 0.5f) {
		Camera::defaultCamera.updateCameraPos(CameraDirection::FORWARD, deltaTime);
	}

	if (ly < -0.05f) {
		Camera::defaultCamera.updateCameraPos(CameraDirection::BACKWARD, deltaTime);
	}

	if (mainJ.buttonState(GLFW_GAMEPAD_BUTTON_A)) {
		Camera::defaultCamera.updateCameraPos(CameraDirection::UP, deltaTime);
	}

	if (mainJ.buttonState(GLFW_GAMEPAD_BUTTON_X)) {
		Camera::defaultCamera.updateCameraPos(CameraDirection::DOWN, deltaTime);
	}

	/*if (std::abs(ry) > 0.5f) {
		z += (ry * speed * deltaTime);
	}*/


	if (std::abs(rx) > 0.1f || std::abs(ry) > 0.1f) {
		double dx = rx * sensitivity * deltaTime;
		double dy = ry * sensitivity * deltaTime;
		Camera::defaultCamera.updateCameraDirection(dx, dy);
	}
}

ArrayObject quadVAO;
void renderQuad(){
	if (!quadInit) {
		quadInit = true;
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		int noElements = sizeof(quadVertices) / sizeof(quadVertices[0]);
		quadVAO.generate();
		quadVAO["VBO"] = BufferObject(GL_ARRAY_BUFFER);
		quadVAO.bind();
		quadVAO["VBO"].bind();
		quadVAO["VBO"].setData<float>(noElements, quadVertices, GL_STATIC_DRAW);
		quadVAO["VBO"].setAttPointer<float>(0, 3, GL_FLOAT, 5, 0);
		quadVAO["VBO"].setAttPointer<float>(1, 2, GL_FLOAT, 5, 3);
	}
	quadVAO.bind();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	ArrayObject::clear();
}
