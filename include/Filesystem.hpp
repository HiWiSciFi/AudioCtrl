#pragma once

#include <string>
#include <filesystem>

class Filesystem {
public:
	std::string readFileText(std::filesystem::path& path);
};
