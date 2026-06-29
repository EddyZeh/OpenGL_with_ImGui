#include "path.h"

void Paths::init() {
	executableDirectory = FileSytem::GetExecutableDirectory();
}

std::filesystem::path Paths::Assets(){
	return executableDirectory / "assets";
}

std::filesystem::path Paths::Shaders(){
	return executableDirectory / "shaders";
}

std::filesystem::path Paths::Textures(){
	return Assets() / "textures";
}

std::filesystem::path Paths::Models(){
	return Assets() / "models";
}

std::filesystem::path Paths::Cubemaps(){
	return Assets() / "cubemaps";
}

