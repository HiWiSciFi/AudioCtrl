#include "Com.hpp"

Com::Com(const std::string& port, DWORD baud) {
	this->com = CreateFile(
		port.c_str(),
		GENERIC_READ | GENERIC_WRITE,
		0,
		nullptr,
		OPEN_EXISTING,
		0,
		nullptr
	);

	if (this->com == INVALID_HANDLE_VALUE) {
		// TODO
	}

	RtlSecureZeroMemory(&this->dcb, sizeof(this->dcb));
	this->dcb.DCBlength = sizeof(this->dcb);

	BOOL success = GetCommState(this->com, &this->dcb);
	// TODO: handle errors

	this->dcb.BaudRate = baud;
	this->dcb.ByteSize = 8;
	this->dcb.Parity = NOPARITY;
	this->dcb.StopBits = ONESTOPBIT;

	success = SetCommState(this->com, &this->dcb);
	// TODO: handle errors

	success = GetCommState(this->com, &this->dcb);
	// TODO: handle errors

	//OVERLAPPED comReader = {
	//	.hEvent = CreateEventA(nullptr, TRUE, FALSE, nullptr)
	//};
	//// TODO: handle errors

	success = SetCommMask(this->com, EV_RXCHAR);
	// TODO: handle errors
}

Com::~Com() {
	BOOL success = CloseHandle(this->com);
	// TODO: handle errors
}

void Com::send(const char* buffer, size_t bufferLen) {
	DWORD bytesWritten = 0;
	BOOL success = WriteFile(this->com, buffer, bufferLen, &bytesWritten, nullptr);
	// TODO: handle errors
	if (bytesWritten != bufferLen) {
		// TODO
	}
}

void Com::receive(std::vector<uint8_t>& buffer) {
	RtlSecureZeroMemory(buffer.data(), buffer.size() * sizeof(std::vector<uint8_t>::value_type));

	DWORD bytesRead = 0;
	//do {
		BOOL success = ReadFile(this->com, buffer.data(), buffer.size(), &bytesRead, nullptr);
		// TODO: handle errors

		buffer.resize(bytesRead);
	//} while(bytesRead > 0);
}

void Com::waitForData() {
	DWORD commEvent = 0;
	BOOL success = WaitCommEvent(this->com, &commEvent, nullptr);
	// TODO: handle errors
}
