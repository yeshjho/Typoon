#include "../../low_level/tray_icon.h"

#include <Windows.h>

#include "../../low_level/filesystem.h"
#include "../../resource.h"
#include "../../utils/config.h"
#include "../../utils/logger.h"
#include "wnd_proc.h"


constexpr UINT WM_TRAY_ICON = WM_USER + 1;

constexpr int IDM_OPEN_CONFIG = 100;
constexpr int IDM_OPEN_MATCH = 101;
constexpr int IDM_EXIT = 1000;


std::optional<LRESULT> tray_icon_proc(HWND hWnd, UINT msg, [[maybe_unused]] WPARAM wParam, LPARAM lParam)
{
    if (msg != WM_TRAY_ICON && msg != WM_COMMAND)
    {
        return std::nullopt;
    }

    if (msg == WM_COMMAND)
    {
        switch (LOWORD(wParam))
        {
        case IDM_OPEN_CONFIG:
            ShellExecute(nullptr, nullptr, get_config_file_path().c_str(), nullptr, nullptr, SW_SHOW);
            break;

        case IDM_OPEN_MATCH:
            ShellExecute(nullptr, nullptr, get_config().matchFilePath.c_str(), nullptr, nullptr, SW_SHOW);
            break;

        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;

        default:
            return std::nullopt;
        }

        return 0;
    }

    switch (LOWORD(lParam))
    {
    case WM_CONTEXTMENU:
    case NIN_KEYSELECT:
    case NIN_SELECT:
    {
        SetForegroundWindow(hWnd);

        const HMENU menu = CreatePopupMenu();

        unsigned int index = 0;
        InsertMenu(menu, index++, MF_BYPOSITION | MF_STRING, IDM_OPEN_CONFIG, L"Open Config");
        InsertMenu(menu, index++, MF_BYPOSITION | MF_STRING, IDM_OPEN_MATCH, L"Open Match");
        InsertMenu(menu, index++, MF_BYPOSITION | MF_MENUBARBREAK, 0, nullptr);
        InsertMenu(menu, index++, MF_BYPOSITION | MF_STRING, IDM_EXIT, L"Exit");

        const UINT flags = TPM_RIGHTBUTTON | (GetSystemMetrics(SM_MENUDROPALIGNMENT) ? TPM_RIGHTALIGN : TPM_LEFTALIGN);
        TrackPopupMenuEx(menu, flags, LOWORD(wParam), HIWORD(wParam), hWnd, nullptr);
        DestroyMenu(menu);

        break;
    }

    default:
        break;
    }

    return 0;
}


void show_tray_icon(const std::any& data)
{
    const auto [hInstance, hWnd] = std::any_cast<std::tuple<HINSTANCE, HWND>>(data);

    const HANDLE icon = LoadImage(hInstance, MAKEINTRESOURCE(IDI_TYPOON_ICON), IMAGE_ICON, 
        GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);

    NOTIFYICONDATAW iconData{};
    iconData.cbSize = sizeof(NOTIFYICONDATAW);
    iconData.hWnd = hWnd;
    iconData.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    iconData.uCallbackMessage = WM_TRAY_ICON;
    iconData.hIcon = static_cast<HICON>(icon);
    iconData.uVersion = NOTIFYICON_VERSION_4;
    std::ranges::copy(L"Typoon", iconData.szTip);

    if (!Shell_NotifyIconW(NIM_ADD, &iconData))
    {
        logger.Log(ELogLevel::ERROR, "Shell_NotifyIconW NIM_ADD failed:", std::system_category().message(static_cast<int>(GetLastError())));
        return;
    }
    if (!Shell_NotifyIconW(NIM_SETVERSION, &iconData))
    {
        logger.Log(ELogLevel::ERROR, "Shell_NotifyIconW NIM_SETVERSION failed:", std::system_category().message(static_cast<int>(GetLastError())));
        return;
    }

    wnd_proc_functions.emplace_back(tray_icon_proc);
}


void remove_tray_icon(const std::any& data)
{
    NOTIFYICONDATAW iconData{};
    iconData.cbSize = sizeof(NOTIFYICONDATAW);
    iconData.hWnd = std::any_cast<HWND>(data);

    Shell_NotifyIconW(NIM_DELETE, &iconData);
}
