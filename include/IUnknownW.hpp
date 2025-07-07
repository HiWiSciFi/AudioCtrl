#pragma once

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
	constexpr operator T* () noexcept;
	~IUnknownW() noexcept;

protected:
	T* data;
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
	}
}

template<typename T>
inline constexpr IUnknownW<T>& IUnknownW<T>::operator=(const IUnknownW<T>& other) noexcept {
	if (this->data != nullptr) {
		this->data->Release();
		this->data = nullptr;
	}

	this->data = other.data;
	if (this->data != nullptr) this->data->AddRef();
	return *this;
}

template<typename T>
inline constexpr IUnknownW<T>& IUnknownW<T>::operator=(IUnknownW<T>&& other) noexcept {
	if (this->data != nullptr) {
		this->data->Release();
		this->data = nullptr;
	}

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
	if (this->data != nullptr) {
		this->data->Release();
		this->data = nullptr;
	}

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
inline constexpr IUnknownW<T>::operator T* () noexcept {
	return this->data;
}

template<typename T>
inline IUnknownW<T>::~IUnknownW() noexcept {
	if (this->data != nullptr) {
		this->data->Release();
		this->data = nullptr;
	}
}
