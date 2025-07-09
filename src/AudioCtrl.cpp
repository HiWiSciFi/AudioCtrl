#include <AudioCtrl.hpp>

#include <Audio.hpp>
#include <Com.hpp>

#include <chrono>
#include <iostream>
#include <stdexcept>
#include <thread>

#include <WinHeaderBegin.hpp>
#include <shellapi.h>
#include <WinHeaderEnd.hpp>

#include <fmtlog/fmtlog.h>

constexpr size_t INITIAL_PORTS_LEN = 16;
constexpr ULONG COMM_PORT_ARDUINO = 5;

constexpr UINT_PTR IDM_MENU_1 = 1;
constexpr UINT_PTR IDM_MENU_2 = 2;
constexpr UINT_PTR IDM_MENU_3 = 3;

#define NOTIFICATION_TRAY_ICON_MSG (WM_USER + 0x100)

// ( ) Read config file
// ( ) Construct Audio Tree with slider assignments
// ( ) Add capability to set all audio recursively via DeviceEnumerator
// (X) Lock DeviceEnumerator to not interfere with notifier? Maybe just fix EX handling...

// ( ) Map Output Device to slider
// ( ) Map EXE Path to slider
// ( ) Map EXE Name to slider

int main() {
	fmtlog::flushOn(fmtlog::LogLevel::ERR);
	fmtlog::setLogFile("./out.log");

	Audio::init();
	DeviceEnumerator deviceEnumerator = DeviceEnumerator();

	Com com = Com("\\\\.\\COM4");

	std::vector<uint8_t> buffer = std::vector<uint8_t>(2);
	std::vector<uint8_t> tempBuf = std::vector<uint8_t>(1);

	com.send(tempBuf);

	while (true) {
		com.waitForData();

		while (true) {
			buffer.resize(2);
			com.receive(buffer);

			if (buffer.size() == 0) break;
			if (buffer.size() != 2) continue;

			if ((buffer[0] >> 7) != 0 || buffer[1] >> 7 != 1) {
				com.receive(tempBuf);
				continue;
			}

			uint8_t slider = 0;
			int value = 0;

			slider = (buffer[0] >> 4) & 0b111;
			value = ((buffer[0] & 0b1111) << 6) | (buffer[1] & 0b111111);

			if (slider == 0) std::cout << std::endl;
			else std::cout << '\t';
			if (slider == 5) com.send(tempBuf);
			std::cout << "Slider " << (int)slider << ": " << value;

			if (slider == 0) {
				deviceEnumerator.setMainVolume(value / 1023.0f);
			}

			//if (slider == 5) std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}
	}
}

#ifdef _WIN32

#include <WinUtil.hpp>

static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	int wmId;
	int wmEvent;

	switch (message) {
		case NOTIFICATION_TRAY_ICON_MSG:
			switch (LOWORD(lParam)) {
				case WM_RBUTTONDOWN:
					POINT clickPoint;
					GetCursorPos(&clickPoint);
					HMENU popMenu = CreatePopupMenu();
					InsertMenuA(popMenu, -1, MF_BYPOSITION | MF_STRING, IDM_MENU_1, "Item 1");
					InsertMenuA(popMenu, -1, MF_BYPOSITION | MF_STRING, IDM_MENU_2, "Item 2");
					InsertMenuA(popMenu, -1, MF_BYPOSITION | MF_STRING, IDM_MENU_3, "Item 3");

					MENUITEMINFOA menuItemInfo = {
						.cbSize = sizeof(MENUITEMINFOA),
						.fMask = MIIM_TYPE | MIIM_ID,
						.fType = MFT_STRING,
						.fState = MFS_UNHILITE,
						.wID = IDM_MENU_1,
						.hSubMenu = nullptr,
						.hbmpChecked = nullptr,
						.hbmpUnchecked = nullptr,
						.dwItemData = 0,
						.dwTypeData = const_cast<char*>("Item 4"),
						.hbmpItem = nullptr,
					};
					InsertMenuItemA(popMenu, -1, TRUE, &menuItemInfo);

					SetForegroundWindow(hWnd);
					TrackPopupMenu(popMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_BOTTOMALIGN, clickPoint.x, clickPoint.y, 0, hWnd, nullptr);
					return TRUE;
			}
			break;
		case WM_COMMAND:
			wmId = LOWORD(wParam);
			wmEvent = HIWORD(wParam);
			switch (wmId) {
				case IDM_MENU_1:
					ShellExecuteW(0, 0, L"https://hiwiscifi.net", 0, 0, SW_SHOW);
					break;
				case IDM_MENU_2:
					ShellExecuteW(0, 0, L"https://wikipedia.org", 0, 0, SW_SHOW);
					break;
				case IDM_MENU_3:
					ShellExecuteW(0, 0, L"https://example.com", 0, 0, SW_SHOW);
					break;
			}
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
	}

	return DefWindowProcW(hWnd, message, wParam, lParam);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance2, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR pCmdLine, _In_ int nCmdShow) {
	fmtlog::flushOn(fmtlog::LogLevel::ERR);
	fmtlog::setLogFile("./out.log");

	HINSTANCE hInstance = GetModuleHandleW(nullptr);

	WCHAR* windowClass = const_cast<WCHAR*>(L"MYAPP");

	HICON icon = (HICON)LoadImageA(nullptr, IDI_APPLICATION, IMAGE_ICON, LR_DEFAULTSIZE, LR_DEFAULTSIZE, LR_SHARED);
	HCURSOR cursor = (HCURSOR)LoadImageA(nullptr, IDC_ARROW, IMAGE_CURSOR, LR_DEFAULTSIZE, LR_DEFAULTSIZE, LR_SHARED);

	WNDCLASSEXW wcex = {
		.cbSize = sizeof(WNDCLASSEXW),
		.style = CS_HREDRAW | CS_VREDRAW,
		.lpfnWndProc = WndProc,
		.cbClsExtra = 0,
		.cbWndExtra = 0,
		.hInstance = hInstance,
		.hIcon = icon,
		.hCursor = cursor,
		.hbrBackground = (HBRUSH)(COLOR_WINDOW+1),
		.lpszMenuName = nullptr,
		.lpszClassName = windowClass,
		.hIconSm = nullptr,
	};

	if (RegisterClassExW(&wcex) == 0) {
		loge("{}", WinUtil::getLastErrorText());
		throw std::runtime_error("Failed to register window class!");
	}

	HWND hwnd = CreateWindowExW(
		0,
		windowClass,
		nullptr,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		nullptr,
		nullptr,
		hInstance,
		nullptr
	);

	if (hwnd == NULL) {
		loge("{}", WinUtil::getLastErrorText());
		throw std::runtime_error("Failed to create window!");
	}

	NOTIFYICONDATAW nid = {
		.cbSize = sizeof(NOTIFYICONDATAW),
		.hWnd = hwnd,
		.uID = 1,
		.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP,
		.uCallbackMessage = NOTIFICATION_TRAY_ICON_MSG,
		.hIcon = icon,
		.szTip = L"Audio Control",
	};

	BOOL success = Shell_NotifyIconW(NIM_ADD, &nid);

	if (!success) {
		throw std::runtime_error("Failed to create Tray Icon");
	}

	MSG msg = { };
	while (GetMessage(&msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return main();
}
#endif
