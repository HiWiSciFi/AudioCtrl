#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#undef WIN32_LEAN_AND_MEAN

#include <string>
#include <vector>

class Com {
public:
	Com(const std::string& port, DWORD baud = CBR_9600);
	~Com();

	void send(const std::vector<uint8_t>& buffer);
	void receive(std::vector<uint8_t>& buffer);

	void waitForData();

protected:
	HANDLE com;
	DCB dcb;
};
