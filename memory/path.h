#ifndef PATH_H
#define PATH_H

#include "FileSystem.h"

class Paths {
public:

	static void init();

	static std::filesystem::path Assets();
	static std::filesystem::path Shaders();
	static std::filesystem::path Textures();
	static std::filesystem::path Models();
	static std::filesystem::path Cubemaps();

private:
	inline static std::filesystem::path executableDirectory;
};

#endif // !PATH_H

