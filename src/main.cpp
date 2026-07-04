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

void processInput(float deltaTime);

const unsigned int SCR_WIDTH  = 800;
const unsigned int SCR_HEIGHT = 600;

//Screen screen;
Scene scene(3, 3, "Graphics Engine", 800, 600);
Camera cam1(glm::vec3(0.0f));

Camera Camera::defaultCamera(glm::vec3(0.0f, 0.0f, 0.0f));

static bool quadInit = false;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

bool blinn = false, shadows = true;

enum PostProcessEffect {
	NONE = 0,
	INVERT,
	GRAYSCALE,
	SHARPEN,
	EDGE,
	SOBEL,
	EMBOSS
};
PostProcessEffect currentEffect = PostProcessEffect::NONE;

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
	Shader shader         (ResourceManager::createShader("object"));
	Shader lampShader     (ResourceManager::createShader("object", std::string("lamp")));
	Shader depthMapShader (ResourceManager::createShader("depthMap", true));
	Shader fboShader      (ResourceManager::createShader("FBO"));
	Shader skyboxShader   (ResourceManager::createShader("skybox"));

	// TEXTURES
	Texture containerTex	(ResourceManager::getTexturePath("container2.png"));
	Texture containerSpec	(ResourceManager::getTexturePath("container2_specular.png"));
	Texture woodTex			(ResourceManager::getTexturePath("wood.png"));
	containerTex.load();
	containerSpec.load();
	woodTex.load();
	std::vector<Texture> textures = { containerTex, containerSpec };

	containerTex.bind();
	

	// Cubemap
	Cubemap skybox;
	skybox.init();
	skybox.loadTextures(ResourceManager::getCubemapPath("nature"));

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

	Model m(glm::vec3(2.0f, 1.5f, -1.0f), glm::vec3(0.01f), true);
	m.loadModel(ResourceManager::getModelPath("m4a1/scene.gltf"));
	Model m1(glm::vec3(2.0f, 1.5f, 0.0f), glm::vec3(0.01f));
	m1.loadModel(ResourceManager::getModelPath("lotr_troll/scene.gltf"));


	ArrayObject VAO;
	VAO.generate();
	VAO["VBO"] = BufferObject(GL_ARRAY_BUFFER);
	VAO.bind();
	VAO["VBO"].bind();
	VAO["VBO"].setData<float>((GLuint)MeshData::Plane.vertices.size(), &MeshData::Plane.vertices[0], GL_STATIC_DRAW);
	VAO["VBO"].setAttPointer<float>(0, 3, GL_FLOAT, 8, 0);
	VAO["VB0"].setAttPointer<float>(1, 3, GL_FLOAT, 8, 3);
	VAO["VB0"].setAttPointer<float>(2, 2, GL_FLOAT, 8, 6);

	ArrayObject fboVAO;
	fboVAO.generate();
	fboVAO["VBO"] = BufferObject(GL_ARRAY_BUFFER);
	fboVAO.bind();
	fboVAO["VBO"].bind();
	fboVAO["VBO"].setData<float>((GLuint)MeshData::Quad.vertices.size(), MeshData::Quad.vertices.data(), GL_STATIC_DRAW);
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

	FramebufferObject fbo(Scene::SCR_WIDTH, Scene::SCR_HEIGHT, GL_COLOR_BUFFER_BIT);
	fbo.generate();
	fbo.allocateAndAttachTexture(GL_COLOR_ATTACHMENT0, GL_RGB, GL_UNSIGNED_BYTE);
	fbo.attachRBO(GL_DEPTH_STENCIL_ATTACHMENT, GL_DEPTH24_STENCIL8);
	FramebufferObject::bindDefault();


	shader.activate();
	shader.setInt("diffuseTexture", 0);

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
		processInput(deltaTime);

		// Start ImGui Frame
		scene.frontendNewFrame();

		// ImGui UI
		

		// SCENE RENDERING
		glEnable(GL_DEPTH_TEST);
		scene.update();
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
			depthMapShader.activate();
			for (unsigned int face = 0; face < 6; ++face)
				depthMapShader.setMat4("shadowMatrices[" + std::to_string(face) + "]", shadowTransforms[face]);
			depthMapShader.setFloat("far_plane", far_plane);
			depthMapShader.set3Float("lightPos", lightPos);
			depthMapShader.setBool("instanced", true);
			roomCube.render(depthMapShader);
			cubes.render(depthMapShader);
			spheres.render(depthMapShader);
			depthMapShader.setBool("instanced", false);
			m.render(depthMapShader);
			m1.render(depthMapShader);
		}
		FramebufferObject::bindDefault();
		// END ==================================

		
		// RESET VIEWPORT
		fbo.bind();
		fbo.setViewport();
		scene.update();

		// 2nd PASS (Rendering normal scene)
		glm::mat4 view = scene.getActiveCamera()->getViewMatrix();
		glm::mat4 projection = glm::mat4(1.0f);
		projection = glm::perspective(glm::radians(scene.getActiveCamera()->getZoom()), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.f);

		skybox.render(skyboxShader, view, projection);

		shader.activate();
		scene.render(shader);
		shader.setInt("shadows", shadows);
		shader.setFloat("far_plane", far_plane);
		shader.setInt("noPointLights", static_cast<int>(lamps.pointLightPos.size()));
		for (int i = 0; i < lamps.pointLightPos.size(); i++) {
			shader.set3Float("pointLights[" + std::to_string(i) + "].position", lamps.pointLightPos[i]);
			shader.set3Float("pointLights[" + std::to_string(i) + "].ambient", Material::white_plastic.ambient);
			shader.set3Float("pointLights[" + std::to_string(i) + "].diffuse", Material::white_plastic.diffuse);
			shader.setFloat("pointLights[" + std::to_string(i) + "].k0", 1.0f);
			shader.setFloat("pointLights[" + std::to_string(i) + "].k1", 0.09f);
			shader.setFloat("pointLights[" + std::to_string(i) + "].k2", 0.032f);
		}

		glActiveTexture(GL_TEXTURE0);
		woodTex.bind();
		for (int i = 0; i < lamps.pointLightPos.size(); i++) {
			glActiveTexture(GL_TEXTURE2 + i);
			shader.setInt("pointDepthMaps[" + std::to_string(i) + "]", i + 2);
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
		m1.render(shader);

		lampShader.activate();
		lampShader.setInt("reverse_normals", 0);
		scene.render(lampShader);
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
		float screenWidth = static_cast<float>(Scene::SCR_WIDTH);
		float screenHeight = static_cast<float>(Scene::SCR_HEIGHT);
		fboShader.setFloat("screenWidth", screenWidth);
		fboShader.setFloat("screenHeight", screenHeight);
		fboShader.setInt("effect", currentEffect);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// Render ImGui
		scene.renderIMGUI();
		// CHECK AND CALL EVENTS AND SWAP THE BUFFERS
		scene.backendNewFrame();
	}
	cubes.cleanup();
	spheres.cleanup();
	roomCube.cleanup();
	lamps.cleanup();
	m.cleanup();
	m1.cleanup();
	glDeleteFramebuffers(1, &depthMapFBO);
	fbo.cleanup();
	skybox.cleanup();

	scene.cleanup();
	return 0;
}

void processInput(float deltaTime){
	// KEYBOARD =============================
	scene.processInput(deltaTime);

	if (Keyboard::keyWentDown(GLFW_KEY_B)) {
		blinn = !blinn;
	}
	if (Keyboard::keyWentDown(GLFW_KEY_Y)) {
		shadows = !shadows;
	}

	if (Keyboard::keyWentDown(GLFW_KEY_0)) {
		currentEffect = PostProcessEffect::NONE;
	}
	if (Keyboard::keyWentDown(GLFW_KEY_1)) {
		currentEffect = PostProcessEffect::INVERT;
	}

	if (Keyboard::keyWentDown(GLFW_KEY_2)) {
		currentEffect = PostProcessEffect::GRAYSCALE;
	}

	if (Keyboard::keyWentDown(GLFW_KEY_3)) {
		currentEffect = PostProcessEffect::SHARPEN;
	}

	if (Keyboard::keyWentDown(GLFW_KEY_4)) {
		currentEffect = PostProcessEffect::EDGE;
	}

	if (Keyboard::keyWentDown(GLFW_KEY_5)) {
		currentEffect = PostProcessEffect::SOBEL;
	}

	if (Keyboard::keyWentDown(GLFW_KEY_6)) {
		currentEffect = PostProcessEffect::EMBOSS;
	}

}