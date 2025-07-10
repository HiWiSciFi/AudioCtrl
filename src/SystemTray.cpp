#include <SystemTray.hpp>

#include <stdexcept>

UINT SystemTray::rollingCallback = 1;
std::map<HWND, SystemTray*> SystemTray::systemTrays = {};

SystemTray::SystemTray() {
	this->hwnd = nullptr;
	this->callId = 0;
}

SystemTray::SystemTray(const std::vector<SysTrayItem>& items, const std::wstring& title) {
	this->items = items;
	this->title = title;

	HINSTANCE instance = GetModuleHandle(nullptr);
	if (instance == nullptr) {
		// TODO
	}

	WCHAR* windowClass = this->title.data();
	HICON icon = static_cast<HICON>(LoadImage(nullptr, IDI_APPLICATION, IMAGE_ICON, LR_DEFAULTSIZE, LR_DEFAULTSIZE, LR_SHARED));
	HCURSOR cursor = static_cast<HCURSOR>(LoadImage(nullptr, IDC_ARROW, IMAGE_CURSOR, LR_DEFAULTSIZE, LR_DEFAULTSIZE, LR_SHARED));

	WNDCLASSEX wc = {
		.cbSize = sizeof(WNDCLASSEX),
		.style = CS_HREDRAW | CS_VREDRAW,
		.lpfnWndProc = SystemTray::WndProc,
		.cbClsExtra = 0,
		.cbWndExtra = 0,
		.hInstance = instance,
		.hIcon = icon,
		.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW) + 1,
		.lpszMenuName = nullptr,
		.lpszClassName = windowClass,
		.hIconSm = nullptr,
	};

	ATOM classAtom = RegisterClassEx(&wc);

	if (classAtom == 0) {
		// TODO
	}

	this->hwnd = CreateWindowEx(
		0,
		MAKEINTATOM(classAtom),
		nullptr,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		nullptr,
		nullptr,
		instance,
		nullptr
	);

	if (this->hwnd == nullptr) {
		// TODO
	}

	this->callId = SystemTray::rollingCallback;
	SystemTray::rollingCallback++;

	SystemTray::systemTrays[this->hwnd] = this;

	NOTIFYICONDATA nid = {
		.cbSize = sizeof(NOTIFYICONDATA),
		.hWnd = this->hwnd,
		.uID = 1,
		.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP,
		.uCallbackMessage = static_cast<UINT>(WM_USER) + this->callId,
		.hIcon = icon
	};

	RtlSecureZeroMemory(&nid.szTip, sizeof(nid.szTip));
	size_t charsToCopy = this->title.size();
	charsToCopy = charsToCopy >= (sizeof(nid.szTip) / sizeof(std::wstring::value_type)) ? (sizeof(nid.szTip) / sizeof(WCHAR)) - 1 : charsToCopy;

	memcpy_s(&nid.szTip, sizeof(nid.szTip), this->title.data(), charsToCopy * sizeof(std::wstring::value_type));

	if (!Shell_NotifyIcon(NIM_ADD, &nid)) {
		// TODO
	}
}

SystemTray::~SystemTray() {
	if (this->hwnd != nullptr) SystemTray::systemTrays.erase(this->hwnd);
}

void SystemTray::invokeMenuItem(WORD menuItem) {
	// TODO
}

bool SystemTray::processContextMenuEvent(WORD menuItem, WORD event) {
	return false;
}

#include <WinUtil.hpp>

void SystemTray::openContextMenu() {
	POINT clickPoint;
	if (GetCursorPos(&clickPoint) == 0) {
		// TODO
	}

	HMENU popup = CreatePopupMenu();
	if (popup == nullptr) {
		// TODO
	}

	for (SysTrayItem& item : this->items) {
		MENUITEMINFO menuItem = {
			.cbSize = sizeof(MENUITEMINFO),
			.fMask = static_cast<UINT>(MIIM_TYPE | (item.type == SysTrayItemType::TEXT ? MIIM_ID : 0)),
			.fType = static_cast<UINT>(item.type == SysTrayItemType::TEXT ? MFT_STRING : MFT_SEPARATOR),
			.fState = MFS_UNHILITE,
			.wID = static_cast<UINT>(item.type == SysTrayItemType::TEXT ? item.id : 0),
			.hSubMenu = nullptr,
			.hbmpChecked = nullptr,
			.hbmpUnchecked = nullptr,
			.dwItemData = 0,
			.dwTypeData = item.text.data(),
			.hbmpItem = nullptr,
		};
		if (InsertMenuItem(popup, -1, TRUE, &menuItem) == 0) {
			// TODO
			std::string err = WinUtil::getLastErrorText();
			__debugbreak();
		}
	}

	// ignore return value
	SetForegroundWindow(this->hwnd);

	UINT popupFlags = TPM_VERNEGANIMATION | TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_BOTTOMALIGN;
	if (TrackPopupMenu(popup, popupFlags, clickPoint.x, clickPoint.y, 0, this->hwnd, nullptr) == 0) {
		// TODO
	}
}

bool SystemTray::processTrayEvent(WORD event) {
	switch (event) {
		case WM_RBUTTONDOWN:
			this->openContextMenu();
			break;
		case WM_LBUTTONUP:
			// TODO: execute default
			__debugbreak();
			for (const auto& item : this->items) {
				if (item.lClick) this->invokeMenuItem(item.id);
			}
			break;
		default:
			return false;
	}
	return true;
}

LRESULT CALLBACK SystemTray::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	SystemTray* tray = nullptr;
	try {
		tray = SystemTray::systemTrays.at(hwnd);
	}
	catch (std::out_of_range) {
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	if (msg > WM_USER) {
		if (tray->processTrayEvent(LOWORD(lParam))) return TRUE;
	} else if (msg == WM_COMMAND) {
		if (tray->processContextMenuEvent(LOWORD(wParam) - WM_USER, HIWORD(wParam))) return TRUE;
	} else if (msg == WM_DESTROY) {
		PostQuitMessage(0);
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}
