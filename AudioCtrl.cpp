#include "AudioCtrl.h"

#include <iostream>

#include <Windows.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>
#include <endpointvolume.h>
#include <audiopolicy.h>
#include <Psapi.h>
#include <strsafe.h>

int main(int argc, char** argv)
{
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
								||  (strSessionName[nameLen - 3] == L'b' && strSessionName[nameLen - 2] == L'a' && strSessionName[nameLen - 1] == L't')
								||  (strSessionName[nameLen - 3] == L'c' && strSessionName[nameLen - 2] == L'o' && strSessionName[nameLen - 1] == L'm'))) {
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
