#pragma once
#include <optional>
#include <vector>

#include <ObjectArray.h>
#include <string>


constexpr UINT CONFIG_CHANGED_MESSAGE = WM_USER + 123;
constexpr UINT CLIPBOARD_PASTE_DONE_MESSAGE = CONFIG_CHANGED_MESSAGE + 1;


using WndProcFunc = std::optional<LRESULT>(*)(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
inline std::vector<std::pair<std::string, WndProcFunc>> wnd_proc_functions;  // NOTE: This is not thread-safe, modify only in the main thread.

LRESULT main_wnd_proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
