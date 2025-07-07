#include <Audio.hpp>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#undef WIN32_LEAN_AND_MEAN

#include <Psapi.h>

static constexpr CLSID CLSID_MMDEVICE_ENUMERATOR  = __uuidof(MMDeviceEnumerator);
static constexpr IID IID_IMMDEVICE_ENUMERATOR     = __uuidof(IMMDeviceEnumerator);
static constexpr IID IID_IAUDIO_SESSION_MANAGER_2 = __uuidof(IAudioSessionManager2);
static constexpr IID IID_IAUDIO_ENDPOINT_VOLUME   = __uuidof(IAudioEndpointVolume);
static constexpr IID IID_IMMNOTIFICATION_CLIENT   = __uuidof(IMMNotificationClient);

static void ReleaseIUnknown(IUnknown* obj) {
	obj->Release();
}

void Audio::init() {
	HRESULT result = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	// TODO: handle result
}

Session::Session() {
	this->sessionControl = nullptr;
	this->sessionControl2 = nullptr;
}

#include <iostream>

Session::Session(IAudioSessionControl* sessionControl) {
	this->sessionControl = std::shared_ptr<IAudioSessionControl>(sessionControl, ReleaseIUnknown);

	IAudioSessionControl2* pSessionControl2 = nullptr;
	HRESULT result = this->sessionControl->QueryInterface<IAudioSessionControl2>(&pSessionControl2);
	// TODO: handle result

	this->sessionControl2 = std::shared_ptr<IAudioSessionControl2>(pSessionControl2, ReleaseIUnknown);

	DWORD procID = 0;
	result = this->sessionControl2->GetProcessId(&procID);
	// TODO: handle result

	HANDLE procHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, procID);
	WCHAR path[MAX_PATH];
	if (GetModuleFileNameExW(procHandle, nullptr, path, MAX_PATH) != 0) {
		CloseHandle(procHandle);
		std::wcout << L"Path: " << path << std::endl;
	}

	ISimpleAudioVolume* audioVolume = nullptr;
	result = this->sessionControl2->QueryInterface<ISimpleAudioVolume>(&audioVolume);
	// TODO: handle result
	this->audioVolume = std::shared_ptr<ISimpleAudioVolume>(audioVolume);
}

void Session::setVolume(float volume) {
	this->audioVolume->SetMasterVolume(volume, nullptr);
}

SessionEnumerator::SessionEnumerator() {
	this->sessionManager = nullptr;
	this->sessionEnumerator = nullptr;
}

SessionEnumerator::SessionEnumerator(IAudioSessionManager2* sessionManager) {
	this->sessionManager = std::shared_ptr<IAudioSessionManager2>(sessionManager, ReleaseIUnknown);

	IAudioSessionEnumerator* pSessionEnumerator = nullptr;
	HRESULT result = this->sessionManager->GetSessionEnumerator(&pSessionEnumerator);
	// TODO: handle result

	this->sessionEnumerator = std::shared_ptr<IAudioSessionEnumerator>(pSessionEnumerator, ReleaseIUnknown);

	this->sessions = std::vector<Session>(this->getSessionCount());

	for (int i = 0; i < this->sessions.size(); i++) {
		IAudioSessionControl* sessionControl = nullptr;
		result = this->sessionEnumerator->GetSession(i, &sessionControl);
		// TODO: handle result
		this->sessions[i] = std::move(Session(sessionControl));
	}
}

int SessionEnumerator::getSessionCount() {
	int sessionCount = 0;
	HRESULT result = this->sessionEnumerator->GetCount(&sessionCount);
	// TODO: handle result

	return sessionCount;
}

Device::Device() {
	this->device = nullptr;
}

Device::Device(IMMDevice* device) {
	this->device = std::shared_ptr<IMMDevice>(device, ReleaseIUnknown);

	IAudioSessionManager2* sessionManager = nullptr;
	HRESULT result = this->device->Activate(IID_IAUDIO_SESSION_MANAGER_2, CLSCTX_ALL, nullptr, (void**)&sessionManager);
	// TODO: handle result

	IAudioEndpointVolume* pEndpointVolume = nullptr;
	result = this->device->Activate(IID_IAUDIO_ENDPOINT_VOLUME, CLSCTX_ALL, nullptr, (void**)&pEndpointVolume);
	// TODO: handle result
	this->endpointVolume = std::shared_ptr<IAudioEndpointVolume>(pEndpointVolume, ReleaseIUnknown);

	this->sessionEnumerator = std::move(SessionEnumerator(sessionManager));
}

void Device::setDeviceVolume(float volume) {
	HRESULT result = this->endpointVolume->SetMasterVolumeLevelScalar(volume, nullptr);
	// TODO: handle result
}

DeviceCollection::DeviceCollection() {
	this->collection = nullptr;
}

DeviceCollection::DeviceCollection(IMMDeviceCollection* collection) {
	this->collection = std::shared_ptr<IMMDeviceCollection>(collection, ReleaseIUnknown);
	this->devices = std::vector<Device>(this->getDeviceCount());

	for (UINT i = 0; i < this->devices.size(); i++) {
		IMMDevice* pDevice = nullptr;
		HRESULT result = this->collection->Item(i, &pDevice);
		// TODO: handle result
		this->devices[i] = std::move(Device(pDevice));
	}
}

UINT DeviceCollection::getDeviceCount() {
	UINT deviceCount = 0;
	HRESULT result = this->collection->GetCount(&deviceCount);
	// TODO: handle result
	return deviceCount;
}

void DeviceCollection::setMainVolume(float volume) {
	for (auto& device : this->devices) {
		device.setDeviceVolume(volume);
	}
}

DeviceEventNotifier::DeviceEventNotifier() {
	this->enumerator = nullptr;
	this->cRef = 1;
}

DeviceEventNotifier::DeviceEventNotifier(DeviceEnumerator* enumerator) {
	this->enumerator = enumerator;
	this->cRef = 1;
}

HRESULT DeviceEventNotifier::OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDefaultDeviceId) {
	this->enumerator->reloadDevices();
	return S_OK;
}

HRESULT DeviceEventNotifier::OnDeviceAdded(LPCWSTR pwstrDeviceId) {
	this->enumerator->reloadDevices();
	return S_OK;
}

HRESULT DeviceEventNotifier::OnDeviceRemoved(LPCWSTR owstrDeviceId) {
	this->enumerator->reloadDevices();
	return S_OK;
}

HRESULT DeviceEventNotifier::OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState) {
	this->enumerator->reloadDevices();
	return S_OK;
}

HRESULT DeviceEventNotifier::OnPropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY key) {
	this->enumerator->reloadDevices();
	return S_OK;
}

ULONG DeviceEventNotifier::AddRef() {
	return InterlockedIncrement(&cRef);
}

ULONG DeviceEventNotifier::Release() {
	ULONG ulRef = InterlockedDecrement(&cRef);
	if (ulRef == 0) {
		delete this;
	}
	return ulRef;
}

HRESULT DeviceEventNotifier::QueryInterface(REFIID riid, void** ppvInterface) {
	if (IID_IUnknown == riid) {
		AddRef();
		*ppvInterface = (IUnknown*)this;
	} else if (riid == IID_IMMNOTIFICATION_CLIENT) {
		AddRef();
		*ppvInterface = (IMMNotificationClient*)this;
	} else {
		*ppvInterface = NULL;
		return E_NOINTERFACE;
	}
	return S_OK;
}

DeviceEnumerator::DeviceEnumerator() {
	IMMDeviceEnumerator* pEnumerator = nullptr;
	HRESULT result = CoCreateInstance(
		CLSID_MMDEVICE_ENUMERATOR, nullptr,
		CLSCTX_ALL, IID_IMMDEVICE_ENUMERATOR,
		(void**)&(pEnumerator)
	);
	// TODO: handle result

	this->eventNotifier = DeviceEventNotifier(this);
	this->enumerator = std::shared_ptr<IMMDeviceEnumerator>(pEnumerator, ReleaseIUnknown);

	this->enumerator->RegisterEndpointNotificationCallback(&this->eventNotifier);

	this->reloadDevices();
}

DeviceEnumerator::~DeviceEnumerator() {
	this->enumerator->UnregisterEndpointNotificationCallback(&this->eventNotifier);
}

void DeviceEnumerator::reloadDevices() {
	IMMDeviceCollection* pDevices = nullptr;
	HRESULT result = this->enumerator->EnumAudioEndpoints(
		eRender,
		DEVICE_STATE_ACTIVE,
		&pDevices
	);
	// TODO: handle result
	this->deviceCollection = std::move(DeviceCollection(pDevices));
}

void DeviceEnumerator::setMainVolume(float volume) {
	this->deviceCollection.setMainVolume(volume);
}
