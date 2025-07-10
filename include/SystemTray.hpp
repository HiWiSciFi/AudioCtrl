#pragma once

#include <map>
#include <string>
#include <vector>

#include <WinHeaderBegin.hpp>
#include <windows.h>
#include <shellapi.h>
#include <WinHeaderEnd.hpp>

enum class SysTrayItemType : uint8_t {
	TEXT,
	SEPARATOR
};

struct SysTrayItem {
	SysTrayItemType type;
	WORD id;
	bool lClick;
	std::wstring text;
};

class SystemTray {
public:
	SystemTray();
	SystemTray(const std::vector<SysTrayItem>& items, const std::wstring& title);
	~SystemTray();

protected:
	std::wstring title;
	std::vector<SysTrayItem> items;
	HWND hwnd;
	WORD callId;

	void invokeMenuItem(WORD menuItem);
	bool processContextMenuEvent(WORD menuItem, WORD event);

	void openContextMenu();
	bool processTrayEvent(WORD event);

	static UINT rollingCallback;
	static std::map<HWND, SystemTray*> systemTrays;

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};
