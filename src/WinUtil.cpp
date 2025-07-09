#include <WinUtil.hpp>

#include <WinHeaderBegin.hpp>
#include <windows.h>
#include <WinHeaderEnd.hpp>

std::string WinUtil::getLastErrorText() {
	DWORD err = GetLastError();

	CHAR* msg = nullptr;
	if (FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		nullptr,
		err,
		0,
		reinterpret_cast<LPSTR>(&msg),
		1,
		nullptr
	) == 0) {
		return std::to_string(err);
	}

	std::string str = std::string(msg);
	LocalFree(msg);
	return str;
}
