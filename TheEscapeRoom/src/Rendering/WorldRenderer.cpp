#include "WorldRenderer.h"

#include <glm/gtc/matrix_transform.hpp>

#include <cmath>
#include <iostream>
#include <set>

#include "memory/ResourceManager.h"	// engine

namespace {
	// mostly steady with a short stutter every few seconds
	float fluorescent(float t) {
		float cycle = std::fmod(t, 9.3f);
		if (cycle > 8.7f)
			return (std::sin(t * 57.0f) + std::sin(t * 23.0f)) > 0.3f ? 1.0f : 0.2f;
		return 0.97f + 0.03f * std::sin(t * 41.0f);
	}

	float pulse(float t) {
		float s = 0.5f + 0.5f * std::sin(t * 2.4f);
		return 0.3f + 0.7f * s * s;
	}

	bool startsWith(const std::string& s, const std::string& prefix) {
		return !prefix.empty() && s.compare(0, prefix.size(), prefix) == 0;
	}
}

// =========================================================================== setup

bool WorldRenderer::init() {
	// shaders: the game's surface + post shaders, the engine's depth shaders
	roomShader.reset(new Shader(ResourceManager::createShader("room")));
	pointDepthShader.reset(new Shader(ResourceManager::createShader("depthMap", true)));
	planarDepthShader.reset(new Shader(ResourceManager::createShader("shadowMap")));
	postShader.reset(new Shader(ResourceManager::createShader("FBO", std::string("post"))));

	primitives.init();

	// textures (from the engine's asset folder)
	whiteTexture = solidTexture(255, 255, 255);
	greyTexture = solidTexture(70, 70, 70);

	struct TexFiles { const char* diffuse; const char* specular; };
	const TexFiles files[(int)TextureId::Count] = {
		{ "brickwall.jpg", nullptr },				// Brick
		{ "wood.png", nullptr },					// Wood
		{ "container2.png", "container2_specular.png" },	// Metal
		{ "bricks2.jpg", nullptr }					// Concrete
	};
	for (const auto& f : files) {
		Texture d(ResourceManager::getTexturePath(f.diffuse));
		d.load();
		diffuseMaps.push_back(d);

		if (f.specular) {
			Texture s(ResourceManager::getTexturePath(f.specular));
			s.load();
			specularMaps.push_back(s);
		}
		else {
			specularMaps.push_back(greyTexture);
		}
	}

	// one-time sampler units
	LightUnits::configureSamplers(*roomShader);
	roomShader->activate();
	roomShader->setInt("material.diffuseTexture", 0);
	roomShader->setInt("material.specularTexture", 1);

	postShader->activate();
	postShader->setInt("screenTexture", 0);

	// the player's torch (engine SpotLight with its own shadow map)
	torch.cutOff = glm::cos(glm::radians(13.0f));
	torch.outerCutOff = glm::cos(glm::radians(21.0f));
	torch.k0 = 1.0f;
	torch.k1 = 0.09f;
	torch.k2 = 0.032f;
	torch.ambient = glm::vec3(0.0f);
	torch.diffuse = glm::vec3(0.95f, 0.92f, 0.85f);
	torch.specular = glm::vec3(0.8f);
	torch.shadowNear = 0.05f;
	torch.shadowFar = 25.0f;
	torch.position = glm::vec3(0.0f);
	torch.direction = glm::vec3(0.0f, 0.0f, -1.0f);
	torch.initShadowMap(1024);

	post.init(Scene::SCR_WIDTH, Scene::SCR_HEIGHT);

	initialised = true;
	return roomShader->id != 0;
}

void WorldRenderer::setupLights(const Level& level) {
	for (auto& pl : pointLights)
		pl.cleanup();
	pointLights.clear();
	pointShadow.clear();
	pointLights.reserve(LightUnits::MAX_POINT_LIGHTS);

	for (const auto& l : level.lights) {
		if ((int)pointLights.size() >= LightUnits::MAX_POINT_LIGHTS)
			break;

		PointLight pl;
		pl.position = l.position;
		pl.ambient = l.ambient;
		pl.diffuse = l.diffuse;
		pl.specular = l.specular;
		pl.k0 = l.k0;
		pl.k1 = l.k1;
		pl.k2 = l.k2;
		pl.shadowNear = 0.05f;
		pl.shadowFar = 25.0f;
		if (l.castsShadow)
			pl.initShadowMap(1024);

		pointLights.push_back(pl);
		pointShadow.push_back(l.castsShadow);
	}
}

Texture WorldRenderer::solidTexture(unsigned char r, unsigned char g, unsigned char b) {
	Texture t;
	t.generate();
	t.bind();
	unsigned char data[4] = { r, g, b, 255 };
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	Texture::setParams(GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT);
	return t;
}

// =========================================================================== per frame

void WorldRenderer::updateLights(Level& level, const FrameParams& frame, std::vector<float>& emissiveScale) {
	emissiveScale.assign(level.props.size(), 1.0f);

	for (size_t i = 0; i < pointLights.size() && i < level.lights.size(); i++) {
		const LevelLight& l = level.lights[i];
		float k = 1.0f;
		if (l.flicker == Flicker::Fluorescent)
			k = fluorescent(frame.time);
		else if (l.flicker == Flicker::Pulse)
			k = pulse(frame.time);

		PointLight& pl = pointLights[i];
		pl.position = l.position;
		pl.ambient = l.ambient;
		pl.diffuse = l.diffuse * k;
		pl.specular = l.specular * k;
		pl.k0 = l.k0;
		pl.k1 = l.k1;
		pl.k2 = l.k2;

		if (l.fixtureProp >= 0 && l.fixtureProp < (int)emissiveScale.size())
			emissiveScale[l.fixtureProp] = k;
	}

	// torch held slightly right of and below the eye, aimed at the crosshair
	glm::vec3 front = glm::normalize(frame.forward);
	glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));
	glm::vec3 up = glm::cross(right, front);
	torch.position = frame.eye + right * 0.22f - up * 0.18f + front * 0.1f;
	glm::vec3 target = frame.eye + front * 8.0f;
	torch.direction = glm::normalize(target - torch.position);
}

void WorldRenderer::render(Scene& scene, Level& level, const FrameParams& frame) {
	currentLevel = &level;
	lastDrawCalls = 0;

	std::vector<float> emissiveScale;
	updateLights(level, frame, emissiveScale);

	glEnable(GL_DEPTH_TEST);

	// ---- 1st pass: shadow maps -----------------------------------------------------------
	if (frame.shadows) {
		for (size_t i = 0; i < pointLights.size(); i++)
			if (pointShadow[i])
				pointLights[i].renderShadowPass(*pointDepthShader, *this);

		if (frame.torchOn)
			torch.renderShadowPass(*planarDepthShader, *this);
	}

	// ---- 2nd pass: the room, into the post-processing buffer ------------------------------
	post.resize(Scene::SCR_WIDTH, Scene::SCR_HEIGHT);
	post.beginSceneCapture();
	scene.setWindowColor(0.01f, 0.01f, 0.012f, 1.0f);
	scene.update();		// engine: clear colour + depth
	glEnable(GL_DEPTH_TEST);

	scene.view = frame.view;
	scene.projection = frame.projection;
	scene.cameraPos = frame.eye;

	Shader& shader = *roomShader;
	scene.render(shader);	// engine: view / projection / viewPos uniforms

	shader.setBool("shadows", frame.shadows);
	shader.setBool("unlit", false);
	shader.setBool("hasDirLight", false);

	shader.setInt("noPointLights", (int)pointLights.size());
	for (int i = 0; i < (int)pointLights.size(); i++) {
		shader.setBool("pointCastsShadow[" + std::to_string(i) + "]", pointShadow[i]);
		pointLights[i].render(shader, i);
	}

	shader.setInt("noSpotLights", frame.torchOn ? 1 : 0);
	if (frame.torchOn)
		torch.render(shader, 0);

	drawProps(shader, level, &frame, &emissiveScale, false);
	drawModels(shader, level, false);

	if (frame.showColliders)
		drawDebug(level, frame);

	// ---- 3rd pass: post-process to the screen ----------------------------------------------
	postShader->activate();
	postShader->setFloat("time", frame.time);
	postShader->setFloat("vignette", 0.55f);
	postShader->setFloat("grain", 0.035f);
	postShader->setFloat("fade", frame.fade);
	postShader->setFloat("flash", frame.flash);
	postShader->set3Float("flashColor", frame.flashColor);
	post.render(*postShader);
}

void WorldRenderer::renderShadowCasters(Shader& depthShader) {
	if (!currentLevel)
		return;

	// thin panels (doors, notes) must cast shadows from both sides
	glDisable(GL_CULL_FACE);
	depthShader.setBool("instanced", false);
	drawProps(depthShader, *currentLevel, nullptr, nullptr, true);
	drawModels(depthShader, *currentLevel, true);
}

void WorldRenderer::applySurface(Shader& shader, const Surface& s, float emissiveScale) {
	if (s.texture == TextureId::None) {
		shader.setBool("noTex", true);
		shader.set3Float("material.diffuseColor", s.diffuse);
		shader.set3Float("material.specularColor", s.specular);
	}
	else {
		int id = (int)s.texture;
		shader.setBool("noTex", false);
		glActiveTexture(GL_TEXTURE0);
		diffuseMaps[id].bind();
		glActiveTexture(GL_TEXTURE1);
		specularMaps[id].bind();
		glActiveTexture(GL_TEXTURE0);
		shader.set3Float("material.diffuseColor", s.diffuse);
		shader.set3Float("material.specularColor", s.specular);
	}

	shader.setFloat("material.shininess", s.shininess);
	shader.setBool("worldUV", s.worldUV);
	shader.set2Float("uvScale", s.uvScale);
	shader.set3Float("emissive", s.emissive * emissiveScale);
}

void WorldRenderer::drawProps(Shader& shader, Level& level, const FrameParams* frame,
	const std::vector<float>* emissiveScale, bool shadowPass) {

	std::set<std::string> replaced;
	for (const auto& slot : level.modelSlots)
		replaced.insert(slot.group);

	float highlight = 0.0f;
	if (frame && !frame->highlightGroup.empty())
		highlight = 0.55f + 0.45f * std::sin(frame->time * 5.0f);

	for (size_t i = 0; i < level.props.size(); i++) {
		const Prop& p = level.props[i];
		if (!p.visible)
			continue;
		if (shadowPass && !p.castsShadow)
			continue;
		if (!replaced.empty() && replaced.count(p.group))
			continue;

		shader.setMat4("model", p.transform);

		if (!shadowPass) {
			applySurface(shader, p.surface, emissiveScale ? (*emissiveScale)[i] : 1.0f);
			bool lit = frame && startsWith(p.group, frame->highlightGroup);
			shader.setFloat("highlight", lit ? highlight : 0.0f);
		}

		primitives.draw(p.shape);
		lastDrawCalls++;
	}
}

void WorldRenderer::drawModels(Shader& shader, Level& level, bool shadowPass) {
	for (const auto& slot : level.modelSlots) {
		// hide the model when every primitive of its group is hidden (e.g. the key was taken)
		bool anyVisible = false, anyProp = false;
		for (const auto& p : level.props) {
			if (p.group == slot.group) {
				anyProp = true;
				anyVisible = anyVisible || p.visible;
			}
		}
		if (anyProp && !anyVisible)
			continue;

		Model* model = getModel(slot);
		if (!model || model->meshes.empty())
			continue;

		shader.setMat4("model", level.anchor(slot.group) * slot.local);

		if (!shadowPass) {
			shader.set3Float("material.diffuseColor", glm::vec3(1.0f));
			shader.set3Float("material.specularColor", glm::vec3(0.5f));
			shader.setFloat("material.shininess", 32.0f);
			shader.setBool("worldUV", false);
			shader.set2Float("uvScale", glm::vec2(1.0f));
			shader.set3Float("emissive", glm::vec3(0.0f));
			shader.setFloat("highlight", 0.0f);
			glActiveTexture(GL_TEXTURE1);
			greyTexture.bind();
			glActiveTexture(GL_TEXTURE0);
		}

		for (auto& mesh : model->meshes) {
			mesh.render(shader);		// engine Mesh: binds its textures / colours and draws
			lastDrawCalls++;
		}
	}
}

Model* WorldRenderer::getModel(const ModelSlot& slot) {
	auto it = modelCache.find(slot.path);
	if (it != modelCache.end())
		return it->second.get();

	std::unique_ptr<Model> model(new Model(glm::vec3(0.0f), glm::vec3(1.0f), slot.noTex));
	model->loadModel(ResourceManager::getModelPath(slot.path));
	if (model->meshes.empty())
		std::cout << "[EscapeRoom] model '" << slot.path << "' has no meshes; keeping the placeholder hidden" << std::endl;

	Model* raw = model.get();
	modelCache[slot.path] = std::move(model);
	return raw;
}

void WorldRenderer::drawDebug(Level& level, const FrameParams& frame) {
	Shader& shader = *roomShader;
	shader.setBool("unlit", true);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	shader.set3Float("unlitColor", glm::vec3(0.1f, 1.0f, 0.2f));
	for (const auto& c : level.colliders) {
		if (!c.enabled)
			continue;
		shader.setMat4("model", Xform::span(c.box.min - glm::vec3(0.005f), c.box.max + glm::vec3(0.005f)));
		primitives.draw(Shape::Cube);
	}

	if (frame.debugBoxes) {
		shader.set3Float("unlitColor", glm::vec3(1.0f, 0.85f, 0.1f));
		for (const auto& b : *frame.debugBoxes) {
			shader.setMat4("model", Xform::span(b.min, b.max));
			primitives.draw(Shape::Cube);
		}
	}

	shader.set3Float("unlitColor", glm::vec3(0.1f, 0.8f, 1.0f));
	shader.setMat4("model", Xform::span(level.exitZone.min, level.exitZone.max));
	primitives.draw(Shape::Cube);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	shader.setBool("unlit", false);
}

// =========================================================================== cleanup

void WorldRenderer::cleanup() {
	if (!initialised)
		return;

	for (auto& kv : modelCache)
		kv.second->cleanup();
	modelCache.clear();

	for (auto& pl : pointLights)
		pl.cleanup();
	pointLights.clear();
	torch.cleanup();

	for (auto& t : diffuseMaps)
		t.cleanup();
	for (size_t i = 0; i < specularMaps.size(); i++)
		if (specularMaps[i].id != greyTexture.id)
			specularMaps[i].cleanup();
	diffuseMaps.clear();
	specularMaps.clear();
	whiteTexture.cleanup();
	greyTexture.cleanup();

	primitives.cleanup();
	post.cleanup();

	roomShader.reset();
	pointDepthShader.reset();
	planarDepthShader.reset();
	postShader.reset();

	initialised = false;
}
