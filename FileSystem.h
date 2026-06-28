#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <windows.h>
#include <filesystem>

namespace FileSytem {
	std::filesystem::path GetExecutableDirectory();
};

#endif // !FILESYSTEM_H

