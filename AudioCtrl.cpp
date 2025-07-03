#include "AudioCtrl.h"

#include <iostream>

#include <Windows.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>
#include <endpointvolume.h>
#include <audiopolicy.h>
#include <Psapi.h>
#include <strsafe.h>

#include <vector>

constexpr auto INITIAL_PORTS_LEN = 16;

constexpr ULONG COMM_PORT_ARDUINO = 5;

int main(int argc, char** argv) {
	std::vector<ULONG> ports = std::vector<ULONG>(INITIAL_PORTS_LEN);

	ULONG portsLen = INITIAL_PORTS_LEN;
	ULONG result = ERROR_SUCCESS;
	ULONG portNumbersFound = 0;
	do {
		memset(ports.data(), 0, portsLen * sizeof(std::vector<ULONG>::value_type));
		result = GetCommPorts(ports.data(), portsLen, &portNumbersFound);
		if (result == ERROR_MORE_DATA) {
			portsLen *= 2;
			ports.resize(portsLen);
		}
	} while (result == ERROR_MORE_DATA);

	if (result != ERROR_SUCCESS) {
		std::cerr << "ERROR" << std::endl;
		return 1;
	}

	ports.resize(portNumbersFound);

	for (int i = 0; i < ports.size(); i++) std::cout << "COM" << ports[i] << std::endl;
	std::cout << std::endl;

	DCB dcb;
	HANDLE hCom;
	BOOL fSuccess;
	TCHAR* pcCommPort = const_cast<TCHAR*>("COM5");

	std::cout << pcCommPort << std::endl;

	hCom = CreateFile(pcCommPort, GENERIC_READ, 0, nullptr, OPEN_EXISTING, 0, nullptr);
	if (hCom == INVALID_HANDLE_VALUE) {
		std::cerr << "ERROR opening COM Port" << std::endl;
		return 2;
	}

	SecureZeroMemory(&dcb, sizeof(dcb));
	dcb.DCBlength = sizeof(dcb);

	fSuccess = GetCommState(hCom, &dcb);
	if (!fSuccess) {
		std::cerr << "GetCommState failed with error " << GetLastError() << "." << std::endl;
		return 3;
	}

	std::cout << "BaudRate = " << (int)dcb.BaudRate
				<< ", ByteSize = " << (int)dcb.ByteSize
				<< ", Parity = " << (int)dcb.Parity
				<< ", StopBits = " << (int)dcb.StopBits
				<< std::endl;

	dcb.BaudRate = CBR_9600;
	dcb.ByteSize = 8;
	dcb.Parity = NOPARITY;
	dcb.StopBits = ONESTOPBIT;

	fSuccess = SetCommState(hCom, &dcb);
	if (!fSuccess) {
		std::cerr << "SetCommState failed with error " << GetLastError() << "." << std::endl;
		return 4;
	}

	fSuccess = GetCommState(hCom, &dcb);
	if (!fSuccess) {
		std::cerr << "GetCommState failed with error " << GetLastError() << "." << std::endl;
		return 3;
	}

	std::cout << "BaudRate = " << (int)dcb.BaudRate
		<< ", ByteSize = " << (int)dcb.ByteSize
		<< ", Parity = " << (int)dcb.Parity
		<< ", StopBits = " << (int)dcb.StopBits
		<< std::endl;

	OVERLAPPED comReader = {
		.hEvent = CreateEventA(nullptr, TRUE, FALSE, nullptr)
	};

	SetCommMask(hCom, EV_RXCHAR);

	DWORD dwCommEvent = 0;
	char lpBuf[512];
	DWORD dwRead = 0;

	while (1) {
		WaitCommEvent(hCom, &dwCommEvent, &comReader);
		if (comReader.hEvent == 0) continue;
		if (WaitForSingleObject(comReader.hEvent, INFINITE) == WAIT_OBJECT_0) {
			do {
				fSuccess = ReadFile(hCom, lpBuf, 512, &dwRead, &comReader);
				if (!fSuccess) {
					if (GetLastError() == ERROR_IO_PENDING) std::cerr << "Error reading the file" << std::endl;
				} else {
					std::string str = std::string(lpBuf, dwRead);
					std::cout << str;
				}
			} while(dwRead > 0);
		}
	}

	CloseHandle(comReader.hEvent);
	CloseHandle(hCom);

	return 0;
}

int test() {
	const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
	const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);

	HRESULT result = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	switch (result) {
	case RPC_E_CHANGED_MODE:
		std::cerr << "Initialization Error: RPC_E_CHANGED_MODE" << std::endl;
		return 5;
	}

	IMMDeviceEnumerator* pEnumerator = nullptr;

	result = CoCreateInstance(
		CLSID_MMDeviceEnumerator, nullptr,
		CLSCTX_ALL, IID_IMMDeviceEnumerator,
		(void**)&pEnumerator
	);

	switch (result) {
	case REGDB_E_CLASSNOTREG:
		std::cerr << "Create Instance Error: REGDB_E_CLASSNOTREG" << std::endl;
		return 6;
	case CLASS_E_NOAGGREGATION:
		std::cerr << "Create Instance Error: CLASS_E_NOAGGREGATION" << std::endl;
		return 7;
	case E_NOINTERFACE:
		std::cerr << "Create Instance Error: E_NOINTERFACE" << std::endl;
		return 8;
	case E_POINTER:
		std::cerr << "Create Instance Error: E_POINTER" << std::endl;
		return 9;
	}

	IMMDeviceCollection* pDevices = nullptr;

	result = pEnumerator->EnumAudioEndpoints(
		eRender,
		DEVICE_STATE_ACTIVE,
		&pDevices
	);

	switch (result) {
	case E_POINTER:
		std::cerr << "Enumeration Error: E_POINTER" << std::endl;
		return 10;
	case E_INVALIDARG:
		std::cerr << "Enumeration Error: E_INVALIDARG" << std::endl;
		return 11;
	case E_OUTOFMEMORY:
		std::cerr << "Enumeration Error: E_OUTOFMEMORY" << std::endl;
		return 12;
	}

	UINT deviceCount = 0;
	pDevices->GetCount(&deviceCount);

	for (UINT i = 0; i < deviceCount; i++) {
		IMMDevice* pDevice = nullptr;
		pDevices->Item(i, &pDevice);

		LPWSTR pwszID = nullptr;
		pDevice->GetId(&pwszID);

		IPropertyStore* pProperties = nullptr;
		pDevice->OpenPropertyStore(STGM_READ, &pProperties);

		PROPVARIANT pvname;
		PropVariantInit(&pvname);

		pProperties->GetValue(PKEY_Device_FriendlyName, &pvname);

		printf("%S\n", pvname.pwszVal);

		/*IAudioEndpointVolume* pInterface = nullptr;
		pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, nullptr, (void**)&pInterface);
		pInterface->SetMasterVolumeLevelScalar(1.0f, nullptr);*/

		IAudioSessionManager2* pInterface = nullptr;
		pDevice->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, nullptr, (void**)&pInterface);

		IAudioSessionEnumerator* pSessionEnumerator = nullptr;
		pInterface->GetSessionEnumerator(&pSessionEnumerator);

		int sessionCount = 0;
		pSessionEnumerator->GetCount(&sessionCount);

		for (int sessionID = 0; sessionID < sessionCount; sessionID++) {
			IAudioSessionControl* session = nullptr;
			pSessionEnumerator->GetSession(sessionID, &session);

			AudioSessionState state;
			session->GetState(&state);
			//if (state != AudioSessionStateActive) continue;
			LPCWSTR stateStr = nullptr;
			switch (state) {
			case AudioSessionStateActive:   stateStr = L"State: Active";   break;
			case AudioSessionStateInactive: stateStr = L"State: Inactive"; break;
			case AudioSessionStateExpired:  stateStr = L"State: Expored";  break;
			}

			IAudioSessionControl2* sessionControl2 = nullptr;
			session->QueryInterface<IAudioSessionControl2>(&sessionControl2);

			LPWSTR strSessionName = nullptr;
			//sessionControl2->GetDisplayName(&strSessionName);

			WCHAR buffer[MAX_PATH]{ };
			wchar_t* fileDescription = nullptr;
			if (strSessionName == nullptr || strSessionName[0] == '\0') {
				DWORD procID = 0;
				sessionControl2->GetProcessId(&procID);

				HANDLE proc = OpenProcess(
					PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
					FALSE,
					procID
				);

				if (proc) {
					//strSessionName = buffer;
					WCHAR pathBuffer[MAX_PATH]{ };
					GetModuleFileNameExW(proc, 0, pathBuffer, MAX_PATH);

					int versionInfoSize = GetFileVersionInfoSizeW(pathBuffer, nullptr);

					auto versionInfo = new BYTE[versionInfoSize];
					//std::unique_ptr<BYTE[]> versionInfo_automatic_cleanup(versionInfo);
					GetFileVersionInfoW(pathBuffer, 0, versionInfoSize, versionInfo);

					struct LANGANDCODEPAGE {
						WORD wLanguage;
						WORD wCodePage;
					} *translationArray;

					UINT translationArrayByteLength = 0;
					VerQueryValueW(versionInfo, L"\\VarFileInfo\\Translation", (LPVOID*)&translationArray, &translationArrayByteLength);
					//LANGID lang = GetSystemDefaultUILanguage();
					LANGID lang = GetUserDefaultLangID();
					for (int k = 0; k < (translationArrayByteLength / sizeof(LANGANDCODEPAGE)); k++) {
						wchar_t fileDescriptionKey[256];
						StringCbPrintfW(
							fileDescriptionKey,
							256,
							L"\\StringFileInfo\\%04x%04x\\FileDescription",
							translationArray[k].wLanguage,
							translationArray[k].wCodePage
						);

						UINT fileDescriptionSize = 0;
						BOOL res = VerQueryValueW(versionInfo, fileDescriptionKey, (LPVOID*)&fileDescription, &fileDescriptionSize);
						strSessionName = fileDescription;
						break;
					}

					CloseHandle(proc);

					if (strSessionName != nullptr) {
						int nameLen = lstrlenW(strSessionName);
						if (nameLen > 4
							&& strSessionName[nameLen - 4] == L'.'
							&& ((strSessionName[nameLen - 3] == L'e' && strSessionName[nameLen - 2] == L'x' && strSessionName[nameLen - 1] == L'e')
								|| (strSessionName[nameLen - 3] == L'b' && strSessionName[nameLen - 2] == L'a' && strSessionName[nameLen - 1] == L't')
								|| (strSessionName[nameLen - 3] == L'c' && strSessionName[nameLen - 2] == L'o' && strSessionName[nameLen - 1] == L'm'))) {
							strSessionName[nameLen - 4] = L'\0';
						}
					}
				}
			}

			ISimpleAudioVolume* audioVolume = nullptr;
			sessionControl2->QueryInterface<ISimpleAudioVolume>(&audioVolume);

			audioVolume->SetMasterVolume(1.0f, nullptr);

			printf("\tSession: \"%S\" %S\n", strSessionName, stateStr);

			audioVolume->Release();
			sessionControl2->Release();
			session->Release();
		}

		CoTaskMemFree(pwszID);
		PropVariantClear(&pvname);
		pSessionEnumerator->Release();
		pInterface->Release();
		pProperties->Release();
		pDevice->Release();
	}

	return 0;
}