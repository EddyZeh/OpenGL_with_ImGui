#include <glad/glad.h>
#include <GLFW/glfw3.h>


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <vector>


// graphics
#include "graphics/scene.h"
#include "graphics/screen.h"
#include "graphics/Shader.h"
#include "graphics/Texture.h"
#include "graphics/Camera.h"
#include "graphics/cubemap.h"
#include "graphics/light.hpp"
#include "graphics/postprocessor.hpp"
#include "graphics/sceneobjects.hpp"
#include "graphics/gridfloor.hpp"


// Memory
#include "memory/FileSystem.h"
#include "memory/ResourceManager.h"
#include "memory/glMemory.hpp"

// io
#include "io/Mouse.h"
#include "io/Keyboard.h"
#include "io/Joystick.h"

void processInput(float deltaTime, PostProcessor& post);
void renderQuad();

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

const unsigned int SHADOW_RESOLUTION = 1024;

Scene scene(3, 3, "Graphics Engine", 800, 600);
Camera cam1(glm::vec3(0.0f));

Camera Camera::defaultCamera(glm::vec3(0.0f, 0.0f, 0.0f));

float deltaTime = 0.0f;
float lastFrame = 0.0f;

bool blinn = false;			// TODO: not read by any shader yet
bool shadows = true;
bool dirLightOn = true;
bool spotLightsOn = true;
bool quadRendered = false;

bool showSkybox = true;		// off falls back to a flat background colour
bool showGrid = true;

const glm::vec3 MONO_BACKGROUND = glm::vec3(0.784f, 0.388f, 0.478f);
const glm::vec3 SKY_BACKGROUND = glm::vec3(0.10f, 0.10f, 0.10f);

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
	Shader shader(ResourceManager::createShader("object"));
	Shader lampShader(ResourceManager::createShader("object", std::string("lamp")));
	Shader pointDepthShader(ResourceManager::createShader("depthMap", true));	// vert + geom + frag, writes a cubemap
	Shader planarDepthShader(ResourceManager::createShader("shadowMap"));		// one 2D depth map, dir + spot
	Shader fboShader(ResourceManager::createShader("FBO"));
	Shader skyboxShader(ResourceManager::createShader("skybox"));
	Shader normalMapShader(ResourceManager::createShader("normal_mapping"));
	Shader gridShader(ResourceManager::createShader("grid"));

	// TEXTURES
	Texture containerTex(ResourceManager::getTexturePath("container2.png"));
	Texture containerSpec(ResourceManager::getTexturePath("container2_specular.png"));
	Texture woodTex(ResourceManager::getTexturePath("wood.png"));
	Texture brickwall(ResourceManager::getTexturePath("brickwall.jpg"));
	Texture brickwall_normal(ResourceManager::getTexturePath("brickwall_normal.jpg"));
	containerTex.load();
	containerSpec.load();
	woodTex.load();
	brickwall.load();
	brickwall_normal.load();

	std::vector<Texture> containerTextures = { containerTex, containerSpec };
	std::vector<Texture> woodTextures = { woodTex };

	// Cubemap
	Cubemap skybox;
	skybox.init();
	skybox.loadTextures(ResourceManager::getCubemapPath("nature"));

	// SCENE CONTENT
	SceneObjects objects;
	objects.init(woodTextures, containerTextures, Material::turquoise);

	objects.roomCube.cubeInstances.push_back({ glm::vec3(0.0f), glm::vec3(10.0f) });

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
		objects.spheres.sphereInstances.push_back({ spheresPos[i], spheresSizes[i] });
	}

	objects.addModel(glm::vec3(2.0f, 1.5f, -1.0f), glm::vec3(0.01f), true,
		ResourceManager::getModelPath("m4a1/scene.gltf"));
	objects.addModel(glm::vec3(2.0f, 1.5f, 0.0f), glm::vec3(0.01f), false,
		ResourceManager::getModelPath("lotr_troll/scene.gltf"));

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
	std::vector<SpotLight*> spotLights;

	pointLights.reserve(LightUnits::MAX_POINT_LIGHTS);
	spotLights.reserve(LightUnits::MAX_SPOT_LIGHTS);

	std::vector<glm::vec3> pointLightPos = {
		glm::vec3(0.0f),
		glm::vec3(0.4f, -2.0f, 2.0f)
	};
	for (const auto& pos : pointLightPos) {
		PointLight pl;
		pl.position = pos;
		pl.ambient = Material::white_plastic.ambient;
		pl.diffuse = Material::white_plastic.diffuse;
		pl.specular = Material::white_plastic.specular;
		pl.k0 = 1.0f;
		pl.k1 = 0.09f;
		pl.k2 = 0.032f;
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
	spotLights.push_back(&torch);

	// SHADOW MAPS (after the light lists are final)
	sun.initShadowMap(SHADOW_RESOLUTION);
	for (auto& pl : pointLights)
		pl.initShadowMap(SHADOW_RESOLUTION);
	for (auto& sl : spotLights)
		sl->initShadowMap(SHADOW_RESOLUTION);

	// lamp markers sit on the point lights
	for (const auto& pl : pointLights)
		objects.lamps.pointLightPos.push_back(pl.position);

	// GROUND
	GridFloor grid;
	grid.init();

	// POST PROCESSING
	PostProcessor post;
	post.init(Scene::SCR_WIDTH, Scene::SCR_HEIGHT);

	// one-time shader state
	LightUnits::configureSamplers(shader);

	fboShader.activate();
	fboShader.setInt("screenTexture", 0);

	skyboxShader.activate();
	skyboxShader.setInt("skybox", 0);

	normalMapShader.activate();
	normalMapShader.setInt("diffuseMap", 0);
	normalMapShader.setInt("normalMap", 1);

	glm::vec3 lightPos(0.5f, 1.0f, 0.3f);

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


		glEnable(GL_DEPTH_TEST);
		
		// 1st PASS: depth maps, one per shadow casting light
		if (dirLightOn)
			sun.renderShadowPass(planarDepthShader, objects);

		for (auto& pl : pointLights)
			pl.renderShadowPass(pointDepthShader, objects);

		if (spotLightsOn) {
			for (auto& sl : spotLights)
				sl->renderShadowPass(planarDepthShader, objects);
		}
		
		// 2nd PASS: the scene, captured into the post-processing buffer
		const glm::vec3 background = showSkybox ? SKY_BACKGROUND : MONO_BACKGROUND;
		scene.setWindowColor(background.r, background.g, background.b, 1.0f);
		
		post.resize(Scene::SCR_WIDTH, Scene::SCR_HEIGHT);
		post.beginSceneCapture();
		scene.update();

		if (showSkybox)
			skybox.render(skyboxShader, scene.view, scene.projection);

		if (showGrid) {
			// cut the grid out under the room so it does not fight the wooden floor
			float hole =  5.0;
			grid.render(gridShader, scene.view, scene.projection,
				scene.cameraPos, -5.0, hole);
		}
		
		shader.activate();
		scene.render(shader);
		shader.setBool("shadows", shadows);

		shader.setBool("hasDirLight", dirLightOn);
		if (dirLightOn)
			sun.render(shader);

		shader.setInt("noPointLights", static_cast<int>(pointLights.size()));
		for (int i = 0; i < (int)pointLights.size(); i++)
			pointLights[i].render(shader, i);

		shader.setInt("noSpotLights", spotLightsOn ? static_cast<int>(spotLights.size()) : 0);
		torch.position = scene.getActiveCamera()->cameraPos;
		torch.direction = scene.getActiveCamera()->cameraFront;
		if (spotLightsOn) {
			for (int i = 0; i < (int)spotLights.size(); i++)
				spotLights[i]->render(shader, i);
		}

		objects.renderAll(shader);
		
		normalMapShader.activate();
		scene.render(normalMapShader);
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians((float)glfwGetTime() * -10.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
		normalMapShader.setMat4("model", model);
		glActiveTexture(GL_TEXTURE0);
		brickwall.bind();
		glActiveTexture(GL_TEXTURE1);
		brickwall_normal.bind();
		renderQuad();

		
		lampShader.activate();
		scene.render(lampShader);
		objects.renderLamps(lampShader);
		
		ArrayObject::clear();

		// 3rd PASS: blit the captured frame to the screen
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
		sl->cleanup();

	post.cleanup();
	skybox.cleanup();
	grid.cleanup();

	scene.cleanup();
	return 0;
}

void processInput(float deltaTime, PostProcessor& post) {
	// KEYBOARD =============================
	scene.processInput(deltaTime);

	if (Keyboard::keyWentDown(GLFW_KEY_B)) {
		blinn = !blinn;
	}
	if (Keyboard::keyWentDown(GLFW_KEY_Y)) {
		shadows = !shadows;
	}
	if (Keyboard::keyWentDown(GLFW_KEY_K)) {
		dirLightOn = !dirLightOn;
	}
	if (Keyboard::keyWentDown(GLFW_KEY_L)) {
		spotLightsOn = !spotLightsOn;
	}

	if (Keyboard::keyWentDown(GLFW_KEY_M)) {
		showSkybox = !showSkybox;
	}
	if (Keyboard::keyWentDown(GLFW_KEY_G)) {
		showGrid = !showGrid;
	}

	post.handleInput();
}

ArrayObject quadVAO;

void renderQuad(){
	if (!quadRendered) {

		// positions
		glm::vec3 pos1(-1.0f, 1.0f, 0.0f);
		glm::vec3 pos2(-1.0f, -1.0f, 0.0f);
		glm::vec3 pos3(1.0f, -1.0f, 0.0f);
		glm::vec3 pos4(1.0f, 1.0f, 0.0f);

		// texture coordinates
		glm::vec2 uv1(0.0f, 1.0f);
		glm::vec2 uv2(0.0f, 0.0f);
		glm::vec2 uv3(1.0f, 0.0f);
		glm::vec2 uv4(1.0f, 1.0f);

		// normal vector
		glm::vec3 nm(0.0f, 0.0f, 1.0f);

		glm::vec3 tangent1, bitangent1;
		glm::vec3 tangent2, bitangent2;

		// triangle
		glm::vec3 edge1 = pos2 - pos1;
		glm::vec3 edge2 = pos3 - pos1;
		glm::vec2 deltaUV1 = uv2 - uv1;
		glm::vec2 deltaUV2 = uv3 - uv1;

		float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);

		tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

		bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

		
		// triangle2
		edge1 = pos3 - pos1;
		edge2 = pos4 - pos1;
		deltaUV1 = uv3 - uv1;
		deltaUV2 = uv4 - uv1;

		f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);

		tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

		bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

		float quadVertices[] = {
			// positions				//normal			//texcoords		// tangent								// bitangent
			pos1.x, pos1.y, pos1.z,		nm.x, nm.y, nm.z,	uv1.x, uv1.y,	tangent1.x, tangent1.y, tangent1.z,		bitangent1.x, bitangent1.y, bitangent1.z,
			pos2.x, pos2.y, pos2.z,		nm.x, nm.y, nm.z,	uv2.x, uv2.y,	tangent1.x, tangent1.y, tangent1.z,		bitangent1.x, bitangent1.y, bitangent1.z,
			pos3.x, pos3.y, pos3.z,		nm.x, nm.y, nm.z,	uv3.x, uv3.y,	tangent1.x, tangent1.y, tangent1.z,		bitangent1.x, bitangent1.y, bitangent1.z,

			pos1.x, pos1.y, pos1.z,		nm.x, nm.y, nm.z,	uv1.x, uv1.y,	tangent2.x, tangent2.y, tangent2.z,		bitangent2.x, bitangent2.y, bitangent2.z,
			pos3.x, pos3.y, pos3.z,		nm.x, nm.y, nm.z,	uv3.x, uv3.y,	tangent2.x, tangent2.y, tangent2.z,		bitangent2.x, bitangent2.y, bitangent2.z,
			pos4.x, pos4.y, pos4.z,		nm.x, nm.y, nm.z,	uv4.x, uv4.y,	tangent2.x, tangent2.y, tangent2.z,		bitangent2.x, bitangent2.y, bitangent2.z,
		};

		quadVAO.generate();
		quadVAO.bind();
		quadVAO["VBO"] = BufferObject(GL_ARRAY_BUFFER);
		quadVAO["VBO"].bind();

		quadVAO["VBO"].setData<float>((GLuint)(sizeof(quadVertices) / sizeof(quadVertices[0])), quadVertices, GL_STATIC_DRAW);
		quadVAO["VBO"].setAttPointer<float>(0, 3, GL_FLOAT, 14, 0);
		quadVAO["VBO"].setAttPointer<float>(1, 3, GL_FLOAT, 14, 3);
		quadVAO["VBO"].setAttPointer<float>(2, 2, GL_FLOAT, 14, 6);
		quadVAO["VBO"].setAttPointer<float>(3, 3, GL_FLOAT, 14, 8);
		quadVAO["VBO"].setAttPointer<float>(4, 3, GL_FLOAT, 14, 11);

		quadRendered = true;
	}

	quadVAO.bind();
	glDrawArrays(GL_TRIANGLES, 0, 6);
	ArrayObject::clear();
}
