#pragma once

#include <audiopolicy.h>
#include <endpointvolume.h>
#include <mmdeviceapi.h>

#include <vector>
#include <memory>

class Audio {
public:
	static void init();
};

class VolumeControl {
public:
	VolumeControl();
	VolumeControl(ISimpleAudioVolume* audioVolume);

protected:
	std::shared_ptr<ISimpleAudioVolume> audioVolume;
};

class Session {
public:
	Session();
	Session(IAudioSessionControl* sessionControl);

protected:
	std::shared_ptr<IAudioSessionControl> sessionControl;
	std::shared_ptr<IAudioSessionControl2> sessionControl2;
	VolumeControl volumeControl;
};

class SessionEnumerator {
public:
	SessionEnumerator();
	SessionEnumerator(IAudioSessionManager2* sessionManager);

	int getSessionCount();

protected:
	std::shared_ptr<IAudioSessionManager2> sessionManager;
	std::shared_ptr<IAudioSessionEnumerator> sessionEnumerator;
	std::vector<Session> sessions;
};

class Device {
public:
	Device();
	Device(IMMDevice* device);

	void setDeviceVolume(float volume);

protected:
	std::shared_ptr<IMMDevice> device;
	std::shared_ptr<IAudioEndpointVolume> endpointVolume;
	SessionEnumerator sessionEnumerator;
};

class DeviceCollection {
public:
	DeviceCollection();
	DeviceCollection(IMMDeviceCollection* collection);

	UINT getDeviceCount();
	void setMainVolume(float volume);

protected:
	std::shared_ptr<IMMDeviceCollection> collection;
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
	std::shared_ptr<IMMDeviceEnumerator> enumerator;
	DeviceEventNotifier eventNotifier;
	DeviceCollection deviceCollection;
};
