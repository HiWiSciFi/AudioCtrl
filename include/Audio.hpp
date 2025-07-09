#pragma once

#include <IUnknownW.hpp>

#ifdef _WIN32
	#include <WinHeaderBegin.hpp>

	#include <audiopolicy.h>
	#include <endpointvolume.h>
	#include <mmdeviceapi.h>

	#include <WinHeaderEnd.hpp>
#endif

#include <vector>
#include <memory>
#include <mutex>

class Audio {
public:
	static void init();
};

class Session {
public:
	Session();
	Session(IUnknownW<IAudioSessionControl> sessionControl);

	void setVolume(float volume);

protected:
	IUnknownW<IAudioSessionControl> sessionControl;
	IUnknownW<IAudioSessionControl2> sessionControl2;
	IUnknownW<ISimpleAudioVolume> audioVolume;
};

class SessionEnumerator {
public:
	SessionEnumerator();
	SessionEnumerator(IUnknownW<IAudioSessionManager2> sessionManager);

	int getSessionCount();

protected:
	IUnknownW<IAudioSessionManager2> sessionManager;
	IUnknownW<IAudioSessionEnumerator> sessionEnumerator;
	std::vector<Session> sessions;
};

class Device {
public:
	Device();
	Device(IUnknownW<IMMDevice> device);

	void setDeviceVolume(float volume);

protected:
	IUnknownW<IMMDevice> device;
	IUnknownW<IAudioEndpointVolume> endpointVolume;
	SessionEnumerator sessionEnumerator;
};

class DeviceCollection {
public:
	DeviceCollection();
	DeviceCollection(IUnknownW<IMMDeviceCollection> collection);

	UINT getDeviceCount();
	void setMainVolume(float volume);

protected:
	IUnknownW<IMMDeviceCollection> collection;
	std::vector<Device> devices;
};

class DeviceEnumerator;

class DeviceEventNotifier : public IMMNotificationClient {
public:
	DeviceEventNotifier();
	DeviceEventNotifier(DeviceEnumerator* enumerator);

	virtual HRESULT OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDefaultDeviceId) override;
	virtual HRESULT OnDeviceAdded(LPCWSTR pwstrDeviceId) override;
	virtual HRESULT OnDeviceRemoved(LPCWSTR owstrDeviceId) override;
	virtual HRESULT OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState) override;
	virtual HRESULT OnPropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY key) override;

	virtual ULONG AddRef() override;
	virtual ULONG Release() override;
	virtual HRESULT QueryInterface(REFIID riid, void** ppvInterface) override;

protected:
	LONG cRef;
	DeviceEnumerator* enumerator;
};

class DeviceEnumerator {
public:
	DeviceEnumerator();
	~DeviceEnumerator();

	void reloadDevices();
	void setMainVolume(float volume);

protected:
	IUnknownW<IMMDeviceEnumerator> enumerator;
	DeviceEventNotifier eventNotifier;
	DeviceCollection deviceCollection;

	std::mutex access_mutex;
};
