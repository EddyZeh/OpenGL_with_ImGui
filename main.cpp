#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui.h>


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "Screen.h"

#include "Shader.h"
#include "Texture.h"
#include "Camera.h"
#include "glMemory.hpp"
#include "framebuffer.hpp"

#include "Mouse.h"
#include "Keyboard.h"
#include "Joystick.h"

#include "cube.hpp"
#include "lamp.hpp"
#include "sphere.hpp"

void processInput(float deltaTime);


const unsigned int SCR_WIDTH  = 800;
const unsigned int SCR_HEIGHT = 600;

Screen screen;

Camera Camera::defaultCamera(glm::vec3(0.0f, 0.0f, 0.0f));
Joystick mainJ(0);

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

	glEnable(GL_DEPTH_TEST);
	screen.setParamters();

	// Create IMGUI context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	
	// Style
	ImGui::StyleColorsDark();


	// Initialize backends
	ImGui_ImplGlfw_InitForOpenGL(screen.window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	// Configure global opengl state
	glEnable(GL_DEPTH_TEST);

	// SHADERS
	Shader shader("object.vert", "shadows.frag");
	Shader lampShader("object.vert", "lamp.frag");
	Shader cubemapShader("cubemap.vert", "cubemap.frag", "cubemap.geom");
	Shader fboShader("FBO.vert", "FBO.frag");

	// TEXTURES
	Texture containerTex("assets", "container2.png");
	Texture containerSpec("assets", "container2_specular.png");
	Texture woodTex("assets", "wood.png");
	containerTex.load();
	containerSpec.load();
	woodTex.load();
	std::vector<Texture> textures = { containerTex, containerSpec };

	containerTex.bind();
	

	// MODELS
	
	CubeArray roomCube;
	std::vector<Texture> tex = { woodTex };
	roomCube.init(tex);
	roomCube.cubeInstances.push_back({glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(10.0f)});


	std::vector<glm::vec3> cubesPos = {
		glm::vec3(0.0f, 1.5f, -1.5f),
		glm::vec3(2.0f, 0.0f, 1.0f),
		glm::vec3(-1.0f, 0.0f, 2.0f)
	};

	std::vector<glm::vec3> cubeSizes = { glm::vec3(1.0f), glm::vec3(0.5f), glm::vec3(0.25f) };

	CubeArray cubes;
	cubes.init(textures);
	for (int i = 0; i < 3; i++) {
		cubes.cubeInstances.push_back({ cubesPos[i], cubeSizes[i] });
	}

	LampArray lamps;
	lamps.init();
	lamps.pointLightPos.push_back(glm::vec3(0.0f));
	lamps.pointLightPos.push_back(glm::vec3(0.4f, -2.0f, 2.0f));
	

	std::vector<glm::vec3> spheresPos = {
		glm::vec3(-1.0f, 1.5f, 1.0f),
		glm::vec3(-2.0f, 0.0f, -1.0f)
	};

	std::vector<glm::vec3> spheresSizes = { glm::vec3(1.0f), glm::vec3(0.5f) };
	SphereArray spheres;
	spheres.init(Material::turquoise);

	for (int i = 0; i < 2; i++) {
		spheres.sphereInstances.push_back({spheresPos[i], spheresSizes[i]});
	}

	Model m(glm::vec3(2.0f, 1.5f, 0.0f), glm::vec3(0.01f));
	m.loadModel("assets/models/lotr_troll/scene.gltf");

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

	float quadVertices[] = {
		// positions		// texCoords
		-1.0f,  1.0f,		0.0f, 1.0f,
		-1.0f, -1.0f,		0.0f, 0.0f,
		 1.0f, -1.0f,		1.0f, 0.0f,

		-1.0f,  1.0f,		0.0f, 1.0f,
		 1.0f, -1.0f,		1.0f, 0.0f,
		 1.0f,  1.0f,		1.0f, 1.0f
	};

	ArrayObject fboVAO;
	fboVAO.generate();
	fboVAO["VBO"] = BufferObject(GL_ARRAY_BUFFER);
	fboVAO.bind();
	fboVAO["VBO"].bind();
	fboVAO["VBO"].setData<float>(sizeof(quadVertices) / sizeof(quadVertices[0]), quadVertices, GL_STATIC_DRAW);
	fboVAO["VBO"].setAttPointer<float>(0, 2, GL_FLOAT, 4, 0);
	fboVAO["VBO"].setAttPointer<float>(1, 2, GL_FLOAT, 4, 2);

	ArrayObject::clear();

	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);

	std::vector<unsigned int> depthCubemaps(lamps.pointLightPos.size());

	for (unsigned int i = 0; i < depthCubemaps.size(); i++) {

		glGenTextures(1, &depthCubemaps[i]);
		glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemaps[i]);
		for (unsigned int j = 0; j < 6; j++) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, 0,
				GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemaps[0], 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	FramebufferObject fbo(screen.SCR_WIDTH, screen.SCR_HEIGHT, GL_COLOR_BUFFER_BIT);
	fbo.generate();
	fbo.allocateAndAttachTexture(GL_COLOR_ATTACHMENT0, GL_RGB, GL_UNSIGNED_BYTE);
	fbo.attachRBO(GL_DEPTH_STENCIL_ATTACHMENT, GL_DEPTH24_STENCIL8);
	FramebufferObject::bindDefault();

	mainJ.update();
	if (mainJ.isPresent()) {
		std::cout << mainJ.getName() << " is present." << std::endl;
	}
	else {
		std::cout << "Not present." << std::endl;
	}

	shader.activate();
	shader.setInt("diffuseTexture", 0);

	fboShader.activate();
	fboShader.setInt("screenTexture", 0);

	// MAIN LOOP
	while (!screen.shouldClose()) {
		// INPUT
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		processInput(deltaTime);

		// Start ImGui Frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// ImGui UI
		

		// RENDERING
		glEnable(GL_DEPTH_TEST);
		screen.update();

		// 1st PASS (Rendering DepthMap) ======================
			
		float aspect = (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT;
		float near_plane = 1.0f, far_plane = 25.0f;
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);

		for (int i = 0; i < lamps.pointLightPos.size(); i++) {

			glm::vec3 lightPos = lamps.pointLightPos.at(i);

			glm::mat4 shadowproj = glm::perspective(glm::radians(90.0f), aspect, near_plane, far_plane);

			std::vector<glm::mat4> shadowTransforms;
			shadowTransforms.push_back(shadowproj *
				glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
			shadowTransforms.push_back(shadowproj *
				glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
			shadowTransforms.push_back(shadowproj *
				glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
			shadowTransforms.push_back(shadowproj *
				glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
			shadowTransforms.push_back(shadowproj *
				glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
			shadowTransforms.push_back(shadowproj *
				glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));

			// Attach light's depth cubemap texture
			glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemaps[i], 0);

			glClear(GL_DEPTH_BUFFER_BIT);
			cubemapShader.activate();
			for (unsigned int face = 0; face < 6; ++face)
				cubemapShader.setMat4("shadowMatrices[" + std::to_string(face) + "]", shadowTransforms[face]);
			cubemapShader.setFloat("far_plane", far_plane);
			cubemapShader.set3Float("lightPos", lightPos);
			cubemapShader.setBool("instanced", true);
			roomCube.render(cubemapShader);
			cubes.render(cubemapShader);
			spheres.render(cubemapShader);
			cubemapShader.setBool("instanced", false);
			m.render(cubemapShader);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		// END ==================================
		
		// RESET VIEWPORT
		fbo.bind();
		fbo.setViewport();
		screen.update();

		// 2nd PASS (Rendering normal scene)
		
		glm::mat4 view = Camera::defaultCamera.getViewMatrix();
		glm::mat4 projection = glm::mat4(1.0f);
		projection = glm::perspective(glm::radians(Camera::defaultCamera.getZoom()), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.f);


		shader.activate();
		shader.setMat4("view", view);
		shader.setMat4("projection", projection);

		shader.set3Float("viewPos", Camera::defaultCamera.cameraPos);
		shader.setInt("shadows", shadows);
		shader.setFloat("far_plane", far_plane);
		shader.setInt("noPointLights", static_cast<int>(lamps.pointLightPos.size()));
		for (int i = 0; i < lamps.pointLightPos.size(); i++) {
			shader.set3Float("pointLights[" + std::to_string(i) + "].position", lamps.pointLightPos[i]);
			shader.set3Float("pointLights[" + std::to_string(i) + "].ambient", Material::white_plastic.ambient);
			shader.set3Float("pointLights[" + std::to_string(i) + "].diffuse", Material::white_plastic.diffuse);
			shader.setFloat("pointLights[" + std::to_string(i) + "].constant", 1.0f);
			shader.setFloat("pointLights[" + std::to_string(i) + "].linear", 0.09f);
			shader.setFloat("pointLights[" + std::to_string(i) + "].quadratic", 0.032f);
		}

		glActiveTexture(GL_TEXTURE0);
		woodTex.bind();
		for (int i = 0; i < lamps.pointLightPos.size(); i++) {
			glActiveTexture(GL_TEXTURE2 + i);
			shader.setInt("depthMaps[" + std::to_string(i) + "]", i + 2);
			glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemaps[i]);
		}

		shader.setBool("instanced", true);
		shader.setInt("reverse_normals", 1);
		roomCube.render(shader);
		shader.setInt("reverse_normals", 0);
		cubes.render(shader);
		spheres.render(shader);
		shader.setBool("instanced", false);
		m.render(shader);

		lampShader.activate();
		lampShader.setMat4("view", view);
		lampShader.setMat4("projection", projection);
		lampShader.setInt("reverse_normals", 0);
		lampShader.setBool("instanced", true);
		lamps.render(lampShader);

		ArrayObject::clear();

		FramebufferObject::bindDefault();
		fbo.clear();
		glDisable(GL_DEPTH_TEST);

		fboShader.activate();
		fboVAO.bind();
		for (unsigned int i = 0; i < fbo.textures.size(); i++) {
			glActiveTexture(GL_TEXTURE0 + i);
			fbo.textures[i].bind();
		}
		fboShader.setInt("screenTexture", 0);
		float screenWidth = static_cast<float>(screen.SCR_WIDTH);
		float screenHeight = static_cast<float>(screen.SCR_HEIGHT);
		fboShader.setFloat("screenWidth", screenWidth);
		fboShader.setFloat("screenHeight", screenHeight);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// Render ImGui
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// CHECK AND CALL EVENTS AND SWAP THE BUFFERS
		screen.newFrame();
	}
	cubes.cleanup();
	spheres.cleanup();
	roomCube.cleanup();
	lamps.cleanup();
	glDeleteFramebuffers(1, &depthMapFBO);
	fbo.cleanup();

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
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
