#pragma once

#ifdef _WIN32
	#include <WinHeaderBegin.hpp>

	#include <windows.h>

	#include <WinHeaderEnd.hpp>
#endif

#include <string>
#include <vector>

class Com {
public:
	Com(const std::wstring& port, DWORD baud = CBR_9600);
	~Com();

	void send(const std::vector<uint8_t>& buffer);
	void receive(std::vector<uint8_t>& buffer);

	void waitForData();

protected:
	HANDLE com;
	DCB dcb;
};
