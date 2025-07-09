#pragma once

#include <string>

class WinUtil final {
public:
	WinUtil() = delete;

	static std::string getLastErrorText();
};
