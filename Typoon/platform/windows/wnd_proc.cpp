#include "wnd_proc.h"


LRESULT main_wnd_proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_DESTROY)
    {
        PostQuitMessage(0);
        return 0;
    }

    for (const WndProcFunc function : wnd_proc_functions)
    {
        if (const std::optional<LRESULT> result = function(hWnd, msg, wParam, lParam))
        {
            return *result;
        }
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}
