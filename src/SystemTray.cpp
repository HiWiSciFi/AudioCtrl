#include <SystemTray.hpp>

#include <WinHeaderBegin.hpp>
#include <windows.h>
#include <WinHeaderEnd.hpp>

static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	return DefWindowProcW(hWnd, msg, wParam, lParam);
}
