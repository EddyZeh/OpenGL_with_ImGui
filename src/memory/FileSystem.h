#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <windows.h>
#include <filesystem>

namespace FileSystem {
	std::filesystem::path GetExecutableDirectory();
};

#endif // !FILESYSTEM_H

