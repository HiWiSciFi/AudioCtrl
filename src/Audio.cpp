#include <Audio.hpp>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#undef WIN32_LEAN_AND_MEAN

#include <Psapi.h>
#include <iostream>

static constexpr CLSID CLSID_MMDEVICE_ENUMERATOR  = __uuidof(MMDeviceEnumerator);
static constexpr IID IID_IMMDEVICE_ENUMERATOR     = __uuidof(IMMDeviceEnumerator);
static constexpr IID IID_IAUDIO_SESSION_MANAGER_2 = __uuidof(IAudioSessionManager2);
static constexpr IID IID_IAUDIO_ENDPOINT_VOLUME   = __uuidof(IAudioEndpointVolume);
static constexpr IID IID_IMMNOTIFICATION_CLIENT   = __uuidof(IMMNotificationClient);

void Audio::init() {
	HRESULT result = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	// TODO: handle result
}

Session::Session() {
	this->sessionControl = nullptr;
	this->sessionControl2 = nullptr;
}

Session::Session(IUnknownW<IAudioSessionControl> sessionControl) {
	this->sessionControl = sessionControl;

	HRESULT result = this->sessionControl->QueryInterface(&this->sessionControl2);
	// TODO: handle result

	DWORD procID = 0;
	result = this->sessionControl2->GetProcessId(&procID);
	// TODO: handle result

	HANDLE procHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, procID);
	WCHAR path[MAX_PATH];
	if (GetModuleFileNameExW(procHandle, nullptr, path, MAX_PATH) != 0) {
		CloseHandle(procHandle);
		std::wcout << L"Path: " << path << std::endl;
	}

	result = this->sessionControl2->QueryInterface(&this->audioVolume);
	// TODO: handle result
}

void Session::setVolume(float volume) {
	this->audioVolume->SetMasterVolume(volume, nullptr);
}

SessionEnumerator::SessionEnumerator() {
	this->sessionManager = nullptr;
	this->sessionEnumerator = nullptr;
}

SessionEnumerator::SessionEnumerator(IUnknownW<IAudioSessionManager2> sessionManager) {
	this->sessionManager = sessionManager;

	HRESULT result = this->sessionManager->GetSessionEnumerator(&this->sessionEnumerator);
	// TODO: handle result

	this->sessions = std::vector<Session>(this->getSessionCount());

	for (int i = 0; i < this->sessions.size(); i++) {
		IUnknownW<IAudioSessionControl> sessionControl;
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

Device::Device(IUnknownW<IMMDevice> device) {
	this->device = device;

	IUnknownW<IAudioSessionManager2> sessionManager;
	HRESULT result = this->device->Activate(IID_IAUDIO_SESSION_MANAGER_2, CLSCTX_ALL, nullptr, reinterpret_cast<void**>(&sessionManager));
	// TODO: handle result

	result = this->device->Activate(IID_IAUDIO_ENDPOINT_VOLUME, CLSCTX_ALL, nullptr, reinterpret_cast<void**>(&this->endpointVolume));
	// TODO: handle result

	this->sessionEnumerator = std::move(SessionEnumerator(sessionManager));
}

void Device::setDeviceVolume(float volume) {
	HRESULT result = this->endpointVolume->SetMasterVolumeLevelScalar(volume, nullptr);
	// TODO: handle result
}

DeviceCollection::DeviceCollection() {
	this->collection = nullptr;
}

DeviceCollection::DeviceCollection(IUnknownW<IMMDeviceCollection> collection) {
	this->collection = collection;
	this->devices = std::vector<Device>(this->getDeviceCount());

	for (UINT i = 0; i < this->devices.size(); i++) {
		IUnknownW<IMMDevice> pDevice;
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
	//this->enumerator->reloadDevices();
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
		*ppvInterface = static_cast<IUnknown*>(this);
	} else if (riid == IID_IMMNOTIFICATION_CLIENT) {
		AddRef();
		*ppvInterface = static_cast<IMMNotificationClient*>(this);
	} else {
		*ppvInterface = nullptr;
		return E_NOINTERFACE;
	}
	return S_OK;
}

DeviceEnumerator::DeviceEnumerator() {
	HRESULT result = CoCreateInstance(
		CLSID_MMDEVICE_ENUMERATOR, nullptr,
		CLSCTX_ALL, IID_IMMDEVICE_ENUMERATOR,
		reinterpret_cast<void**>(&this->enumerator)
	);
	// TODO: handle result

	this->eventNotifier = DeviceEventNotifier(this);
	result = this->enumerator->RegisterEndpointNotificationCallback(&this->eventNotifier);
	// TODO: handle result

	this->reloadDevices();
}

DeviceEnumerator::~DeviceEnumerator() {
	HRESULT result = this->enumerator->UnregisterEndpointNotificationCallback(&this->eventNotifier);
	// TODO: handle result
}

void DeviceEnumerator::reloadDevices() {
	IUnknownW<IMMDeviceCollection> pDevices;
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
