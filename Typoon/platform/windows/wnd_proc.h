#pragma once
#include <optional>
#include <vector>

#include <ObjectArray.h>


using WndProcFunc = std::optional<LRESULT>(*)(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
inline std::vector<WndProcFunc> wnd_proc_functions;  // NOTE: This is not thread-safe, push only in the main thread.

LRESULT main_wnd_proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
