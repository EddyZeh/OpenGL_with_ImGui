#include <glad/glad.h>
#include <GLFW/glfw3.h>


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>


// graphics
#include "graphics/scene.h"
#include "graphics/Screen.h"
#include "graphics/Shader.h"
#include "graphics/Texture.h"
#include "graphics/Camera.h"
#include "graphics/framebuffer.hpp"
#include "graphics/cubemap.h"
#include "graphics/light.hpp"
#include "graphics/postprocessor.hpp"
#include "graphics/sceneobjects.hpp"


// Memory
#include "memory/FileSystem.h"
#include "memory/ResourceManager.h"
#include "memory/glMemory.hpp"

// io
#include "io/Mouse.h"
#include "io/Keyboard.h"
#include "io/Joystick.h"

// primitive shape models
#include "graphics/models/cube.hpp"
#include "graphics/models/lamp.hpp"
#include "graphics/models/sphere.hpp"

void processInput(float deltaTime, PostProcessor &post);

const unsigned int SCR_WIDTH  = 800;
const unsigned int SCR_HEIGHT = 600;

const unsigned int SHADOW_RESOLUTION = 1024;

//Screen screen;
Scene scene(3, 3, "Graphics Engine", 800, 600);
Camera cam1(glm::vec3(0.0f));


float deltaTime = 0.0f;
float lastFrame = 0.0f;

bool blinn = false, shadows = true;
bool dirLightOn = true, spotLightOn = true;

int main() {
	Paths::init();

	if (!scene.init()) {
		std::cout << "Failed to initialize GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	// SCENE SET UP (INIT FRONTEND + CAMERAS + BG COLOR)
	scene.initFrontEnd();

	if (scene.joystickPresent()) {
		std::cout << scene.mainJ.getName() << " is present." << std::endl;
	}
	else {
		std::cout << "Not present." << std::endl;
	}

	scene.setWindowColor(0.1f, 0.1f, 0.1f, 1.0f);
	scene.addCam(&cam1);

	// SHADERS
	Shader shader            (ResourceManager::createShader("object"));
	Shader lampShader        (ResourceManager::createShader("object", std::string("lamp")));
	Shader depthMapShader    (ResourceManager::createShader("depthMap", true));
	Shader planarDepthShader (ResourceManager::createShader("shadowMap"));
	Shader fboShader         (ResourceManager::createShader("FBO"));
	Shader skyboxShader      (ResourceManager::createShader("skybox"));

	// TEXTURES
	Texture containerTex	(ResourceManager::getTexturePath("container2.png"));
	Texture containerSpec	(ResourceManager::getTexturePath("container2_specular.png"));
	Texture woodTex			(ResourceManager::getTexturePath("wood.png"));
	containerTex.load();
	containerSpec.load();
	woodTex.load();
	std::vector<Texture> textures = { containerTex, containerSpec };
	std::vector<Texture> tex = { woodTex };

	containerTex.bind();
	

	// Cubemap
	Cubemap skybox;
	skybox.init();
	skybox.loadTextures(ResourceManager::getCubemapPath("nature"));

	// SCENE CONTENT
	SceneObjects objects;
	objects.init(tex, textures, Material::turquoise);
	objects.roomCube.cubeInstances.push_back({ glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(10.0f) });

	std::vector<glm::vec3> cubesPos = {
		glm::vec3(0.0f, 1.5f, -1.5f),
		glm::vec3(2.0f, 0.0f, 1.0f),
		glm::vec3(-1.0f, 0.0f, 2.0f)
	};

	std::vector<glm::vec3> cubeSizes = { glm::vec3(1.0f), glm::vec3(0.5f), glm::vec3(0.25f) };
	for (int i = 0; i < 3; i++) {
		objects.cubes.cubeInstances.push_back({ cubesPos[i], cubeSizes[i] });
	}

	std::vector<glm::vec3> spheresPos = {
		glm::vec3(-1.0f, 1.5f, 1.0f),
		glm::vec3(-2.0f, 0.0f, -1.0f)
	};

	std::vector<glm::vec3> spheresSizes = { glm::vec3(1.0f), glm::vec3(0.5f) };

	for (int i = 0; i < 2; i++) {
		objects.spheres.sphereInstances.push_back({spheresPos[i], spheresSizes[i]});
	}

	objects.addModel(glm::vec3(2.0f, 1.5f, -1.0f), glm::vec3(0.01f), true, 
					 ResourceManager::getModelPath("m4a1/scene.gltf"));
	objects.addModel(glm::vec3(2.0f, 1.5f, 0.0f), glm::vec3(0.01f), false, 
					 ResourceManager::getModelPath("lotr_troll/scene.gltf"));

	// LIGHTS
	DirLight sun;
	sun.direction = glm::vec3(-0.3f, -1.0f, -0.3f);
	sun.ambient = glm::vec3(0.05f);
	sun.diffuse = glm::vec3(0.35f);
	sun.specular = glm::vec3(0.4f);
	sun.shadowTarget = glm::vec3(0.0f);
	sun.shadowDistance = 4.0f;
	sun.shadowOrthoSize = 7.0f;
	sun.shadowNear = 0.1f;
	sun.shadowFar = 20.0f;

	std::vector<PointLight> pointLights;
	std::vector<SpotLight> spotLights;

	pointLights.reserve(LightUnits::MAX_POINT_LIGHTS);
	spotLights.reserve(LightUnits::MAX_SPOT_LIGHTS);

	std::vector<glm::vec3> pointLightPos = {
		glm::vec3(0.0f),
		glm::vec3(0.4f, -2.0f, 2.0f)
	};

	for (const auto& pos : pointLightPos) {
		PointLight pl = PointLight{pos, Material::white_plastic.ambient, 
			Material::white_plastic.diffuse, Material::white_plastic.specular, 
			1.0f, 0.09f, 0.032f};
		pointLights.push_back(pl);
	}

	SpotLight torch;
	torch.position = scene.getActiveCamera()->cameraPos;
	torch.direction = scene.getActiveCamera()->cameraFront;
	torch.cutOff = glm::cos(glm::radians(12.5f));
	torch.outerCutOff = glm::cos(glm::radians(20.0f));
	torch.k0 = 1.0f;
	torch.k1 = 0.09f;
	torch.k2 = 0.032f;
	torch.ambient = glm::vec3(0.0f);
	torch.diffuse = glm::vec3(1.0f);
	torch.specular = glm::vec3(1.0f);
	spotLights.push_back(torch);

	// SHADOW MAPS 
	sun.initShadowMap(SHADOW_RESOLUTION);
	for (auto& pl : pointLights)
		pl.initShadowMap(SHADOW_RESOLUTION);
	for (auto& sl : spotLights)
		sl.initShadowMap(SHADOW_RESOLUTION);

	for (const auto& pl : pointLights)
		objects.lamps.pointLightPos.push_back(pl.position);

	PostProcessor post;
	post.init(Scene::SCR_WIDTH, Scene::SCR_HEIGHT);

	LightUnits::configureSamplers(shader);

	fboShader.activate();
	fboShader.setInt("screenTexture", 0);

	skyboxShader.activate();
	skyboxShader.setInt("skybox", 0);

	// MAIN LOOP
	while (!scene.shouldClose()) {
		// INPUT
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		processInput(deltaTime, post);

		// Start ImGui Frame
		scene.frontendNewFrame();

		// ImGui UI
		

		// SCENE RENDERING
		glEnable(GL_DEPTH_TEST);
		scene.update();		// 1st PASS (Rendering DepthMap) ======================

		if (dirLightOn)
			sun.renderShadowPass(planarDepthShader, objects);

		for (auto& pl : pointLights)
			pl.renderShadowPass(depthMapShader, objects);

		if (spotLightOn) {
			for (auto& sl : spotLights)
				sl.renderShadowPass(planarDepthShader, objects);
		}
		// END ==================================

		
		// RESET VIEWPORT

		post.beginSceneCapture();
		scene.update();

		// 2nd PASS (Rendering normal scene)
		glm::mat4 view = scene.getActiveCamera()->getViewMatrix();
		glm::mat4 projection = glm::mat4(1.0f);
		projection = glm::perspective(glm::radians(scene.getActiveCamera()->getZoom()), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.f);

		skybox.render(skyboxShader, view, projection);

		shader.activate();
		scene.render(shader);
		shader.setInt("shadows", shadows);

		shader.setBool("hasDirLight", dirLightOn);
		if (dirLightOn)
			sun.render(shader);

		shader.setInt("noPointLights", static_cast<int>(pointLights.size()));
		for (int i = 0; i < (int)pointLights.size(); i++)
			pointLights[i].render(shader, i);

		shader.setInt("noSpotLights", static_cast<int>(spotLights.size()));
		if (spotLightOn) {
			for (int i = 0; i < (int)spotLights.size(); i++)
				spotLights[i].render(shader, i);
		}

		objects.renderAll(shader);

		lampShader.activate();
		scene.render(lampShader);
		objects.renderLamps(lampShader);

		ArrayObject::clear();

		post.render(fboShader);

		// Render ImGui
		scene.renderIMGUI();
		// CHECK AND CALL EVENTS AND SWAP THE BUFFERS
		scene.backendNewFrame();
	}

	objects.cleanup();
	sun.cleanup();
	for (auto& pl : pointLights)
		pl.cleanup();
	for (auto& sl : spotLights)
		sl.cleanup();
	skybox.cleanup();
	scene.cleanup();
	return 0;
}

void processInput(float deltaTime, PostProcessor &post){
	// KEYBOARD =============================
	scene.processInput(deltaTime);

	if (Keyboard::keyWentDown(GLFW_KEY_B)) {
		blinn = !blinn;
	}
	if (Keyboard::keyWentDown(GLFW_KEY_Y)) {
		shadows = !shadows;
	}

	if (Keyboard::keyWentDown(GLFW_KEY_K))
		dirLightOn = !dirLightOn;

	if (Keyboard::keyWentDown(GLFW_KEY_L))
		spotLightOn = !spotLightOn;

	post.handleInput();
}