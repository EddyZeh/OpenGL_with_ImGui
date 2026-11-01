#ifndef ESCAPE_WORLDRENDERER_H
#define ESCAPE_WORLDRENDERER_H

#include <glm/glm.hpp>

#include <map>
#include <memory>
#include <string>
#include <vector>

// engine
#include "graphics/scene.h"
#include "graphics/Shader.h"
#include "graphics/Texture.h"
#include "graphics/light.hpp"
#include "graphics/model.h"
#include "graphics/postprocessor.hpp"

#include "Primitives.h"
#include "../Game/Level.h"

/*
	Draws a Level with the engine's renderer pieces:
	  - engine Shader / Texture / Mesh / Model (Assimp) / PointLight / SpotLight / shadow maps
	  - engine PostProcessor for the off-screen buffer
	The renderer only reads gameplay state; it never decides what is open or locked.
*/

struct FrameParams {
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);
	glm::vec3 eye = glm::vec3(0.0f);
	glm::vec3 forward = glm::vec3(0.0f, 0.0f, -1.0f);

	bool torchOn = true;
	bool shadows = true;
	std::string highlightGroup;		// props whose group starts with this are highlighted
	float time = 0.0f;

	bool showColliders = false;
	const std::vector<Collision::AABB>* debugBoxes = nullptr;	// interaction volumes

	// post
	float fade = 0.0f;
	float flash = 0.0f;
	glm::vec3 flashColor = glm::vec3(1.0f, 0.0f, 0.0f);
};

class WorldRenderer {
public:
	bool init();
	void setupLights(const Level& level);
	void render(Scene& scene, Level& level, const FrameParams& frame);
	void cleanup();

	// called by the engine's light shadow passes (template "Renderable" interface)
	void renderShadowCasters(Shader& depthShader);

	int drawCalls() const { return lastDrawCalls; }

private:
	std::unique_ptr<Shader> roomShader;
	std::unique_ptr<Shader> pointDepthShader;
	std::unique_ptr<Shader> planarDepthShader;
	std::unique_ptr<Shader> postShader;

	Primitives primitives;

	std::vector<Texture> diffuseMaps;
	std::vector<Texture> specularMaps;
	Texture whiteTexture;
	Texture greyTexture;

	std::vector<PointLight> pointLights;
	std::vector<bool> pointShadow;
	SpotLight torch;
	PostProcessor post;

	std::map<std::string, std::unique_ptr<Model>> modelCache;

	Level* currentLevel = nullptr;
	int lastDrawCalls = 0;
	bool initialised = false;

	void updateLights(Level& level, const FrameParams& frame, std::vector<float>& emissiveScale);
	void drawProps(Shader& shader, Level& level, const FrameParams* frame,
		const std::vector<float>* emissiveScale, bool shadowPass);
	void drawModels(Shader& shader, Level& level, bool shadowPass);
	void drawDebug(Level& level, const FrameParams& frame);
	void applySurface(Shader& shader, const Surface& s, float emissiveScale);
	Model* getModel(const ModelSlot& slot);

	static Texture solidTexture(unsigned char r, unsigned char g, unsigned char b);
};

#endif // !ESCAPE_WORLDRENDERER_H
