#pragma once

#include <audiopolicy.h>
#include <endpointvolume.h>
#include <mmdeviceapi.h>

#include <vector>
#include <memory>

template<typename T>
class IUnknownW {
public:
	constexpr IUnknownW() noexcept;
	constexpr IUnknownW(T*) noexcept;
	constexpr IUnknownW(const IUnknownW<T>&) noexcept;
	constexpr IUnknownW(IUnknownW<T>&&) noexcept;
	constexpr IUnknownW(T*&&) noexcept;
	constexpr IUnknownW& operator=(const IUnknownW<T>&) noexcept;
	constexpr IUnknownW& operator=(IUnknownW<T>&&) noexcept;
	constexpr IUnknownW& operator=(T*&&) noexcept;
	constexpr T& operator*();
	constexpr T* operator->();
	constexpr T** operator&() noexcept;
	constexpr operator T*() noexcept;
	~IUnknownW() noexcept;

protected:
	T* data;
};

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
};

////////////////////////////////////
////////// IMPLEMENTATION //////////
////////////////////////////////////

template<typename T>
inline constexpr IUnknownW<T>::IUnknownW() noexcept {
	this->data = nullptr;
}

template<typename T>
inline constexpr IUnknownW<T>::IUnknownW(T* data) noexcept {
	this->data = data;
	if (this->data != nullptr) this->data->AddRef();
}

template<typename T>
inline constexpr IUnknownW<T>::IUnknownW(const IUnknownW<T>& other) noexcept {
	this->data = other.data;
	if (this->data != nullptr) this->data->AddRef();
}

template<typename T>
inline constexpr IUnknownW<T>::IUnknownW(IUnknownW<T>&& other) noexcept {
	this->data = other.data;
	if (this->data != nullptr) {
		this->data->AddRef();
		other.data->Release();
		other.data = nullptr;
	}
}

template<typename T>
inline constexpr IUnknownW<T>::IUnknownW(T*&& other) noexcept {
	this->data = other;
	if (this->data != nullptr) {
		this->data->AddRef();
		other->Release();
	}
}

template<typename T>
inline constexpr IUnknownW<T>& IUnknownW<T>::operator=(const IUnknownW<T>& other) noexcept {
	this->data = other.data;
	if (this->data != nullptr) this->data->AddRef();
	return *this;
}

template<typename T>
inline constexpr IUnknownW<T>& IUnknownW<T>::operator=(IUnknownW<T>&& other) noexcept {
	this->data = other.data;
	if (this->data != nullptr) {
		this->data->AddRef();
		other.data->Release();
		other.data = nullptr;
	}
	return *this;
}

template<typename T>
inline constexpr IUnknownW<T>& IUnknownW<T>::operator=(T*&& other) noexcept {
	this->data = other;
	if (this->data != nullptr) {
		this->data->AddRef();
		other->Release();
	}
	return *this;
}

template<typename T>
inline constexpr T& IUnknownW<T>::operator*() {
	return *(this->data);
}

template<typename T>
inline constexpr T* IUnknownW<T>::operator->() {
	return this->data;
}

template<typename T>
inline constexpr T** IUnknownW<T>::operator&() noexcept {
	return &this->data;
}

template<typename T>
inline constexpr IUnknownW<T>::operator T*() noexcept {
	return this->data;
}

template<typename T>
inline IUnknownW<T>::~IUnknownW() noexcept {
	if (this->data != nullptr) {
		this->data->Release();
		this->data = nullptr;
	}
}
