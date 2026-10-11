#ifndef SCENEOBJECTS_HPP
#define SCENEOBJECTS_HPP

#include <memory>
#include <string>
#include <vector>

#include "Shader.h"
#include "Texture.h"
#include "material.h"
#include "model.h"

#include "models/cube.hpp"
#include "models/lamp.hpp"
#include "models/sphere.hpp"

struct SceneObjects {
	CubeArray roomCube;
	CubeArray cubes;
	SphereArray spheres;
	LampArray lamps;

	std::vector<std::unique_ptr<Model>> models;

	void init(const std::vector<Texture>& roomTextures,
			 const std::vector<Texture>& cubeTextures, Material sphereMaterial) {
		roomCube.init(roomTextures);
		cubes.init(cubeTextures);
		spheres.init(sphereMaterial);
		lamps.init();
	}

	Model& addModel(glm::vec3 pos, glm::vec3 size, bool noTex, const std::string& path) {
		models.push_back(std::unique_ptr<Model>(new Model(pos, size, noTex)));
		models.back()->loadModel(path);
		return *models.back();
	}

	void renderShadowCasters(Shader& shader) {
		shader.setBool("instanced", true);
		roomCube.render(shader);
		cubes.render(shader);
		spheres.render(shader);

		shader.setBool("instanced", false)
			;
		for (auto& model : models)
			model->render(shader);
	}

	// the color pass
	void renderAll(Shader& shader) {
		shader.setBool("instanced", true);
		shader.setInt("reverse_normals", 1);
		roomCube.render(shader);

		shader.setInt("reverse_normals", 0);
		cubes.render(shader);
		spheres.render(shader);

		shader.setBool("instanced", false);
		for (auto& model : models)
			model->render(shader);
	}

	void renderLamps(Shader& shader) {
		shader.setInt("reverse_normals", 0);
		shader.setBool("instanced", true);
		lamps.render(shader);
	}

	void cleanup() {
		roomCube.cleanup();
		cubes.cleanup();
		spheres.cleanup();
		lamps.cleanup();

		for (auto& model : models)
			model->cleanup();
		models.clear();
	}
};

#endif // !SCENEOBJECTS_HPP
