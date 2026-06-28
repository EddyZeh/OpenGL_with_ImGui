#include "FileSystem.h"


std::filesystem::path FileSytem::GetExecutableDirectory(){
	char buffer[MAX_PATH];

	GetModuleFileNameA(nullptr, buffer, MAX_PATH);

	return std::filesystem::path(buffer).parent_path();
}
