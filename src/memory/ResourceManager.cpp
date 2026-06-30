#include "ResourceManager.h"


Shader ResourceManager::createShader(const std::string& name, bool hasGeomShader){
	if (hasGeomShader)
		return createShader(name, name, name);
	return createShader(name, name);
}


/**
* @brief Explicity cast second argument to string to avoid degrading into a boolean if using two differently named shader
* @param fs cast explicity to string
*/
Shader ResourceManager::createShader(const std::string& vs, const std::string& fs){
	auto vert = Paths::Shaders() / (vs + ".vert");
	auto frag = Paths::Shaders() / (fs + ".frag");

	return Shader(vert.string().c_str(), frag.string().c_str());
}

Shader ResourceManager::createShader(const std::string& vs, const std::string& fs, const std::string &gs){
	auto vert = Paths::Shaders() / (vs + ".vert");
	auto frag = Paths::Shaders() / (fs + ".frag");
	auto geom = Paths::Shaders() / (gs + ".geom");

	return Shader(vert.string().c_str(), frag.string().c_str(), geom.string().c_str());
}

std::string ResourceManager::getTexturePath(const std::string& name){
	return (Paths::Textures() / name).string();
}

std::string ResourceManager::getCubemapPath(const std::string& name){
	return (Paths::Cubemaps() / name).string();
}

std::string ResourceManager::getModelPath(const std::string& name){
	return (Paths::Models() / name).string();
}




