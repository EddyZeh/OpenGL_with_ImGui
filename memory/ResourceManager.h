#pragma once

#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include "../graphics/Shader.h"
#include "../graphics/Texture.h"
#include "path.h"

class ResourceManager {
public:
	static Shader createShader(const std::string& name, bool hasGeomShader = false);
	static Shader createShader(const std::string& vs, const std::string& fs);
	static Shader createShader(const std::string& vs, const std::string& fs, const std::string& gs);

	static std::string getTexturePath(const std::string& name);

	static std::string getModelPath(const std::string& name);
};


#endif // !RESOURCE_MANAGER_H

