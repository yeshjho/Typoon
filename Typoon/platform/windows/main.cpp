#include <Windows.h>

#include "../../low_level/file_change_watcher.h"
#include "../../low_level/filesystem.h"
#include "../../low_level/hotkey.h"
#include "../../low_level/tray_icon.h"

#include "../../common/common.h"
#include "../../match/trigger_tree.h"
#include "../../utils/config.h"
#include "../../utils/logger.h"

#include "log.h"
#include "wnd_proc.h"


constexpr UINT CONFIG_CHANGED_MESSAGE = WM_USER + 123;

int wWinMain(HINSTANCE hInstance, [[maybe_unused]] HINSTANCE hPrevInstance, [[maybe_unused]] LPWSTR cmdLine, [[maybe_unused]] int cmdShow)
{
#ifdef _DEBUG
    AllocConsole();
    FILE* fDummy;
    freopen_s(&fDummy, "CONOUT$", "w", stdout);
    freopen_s(&fDummy, "CONOUT$", "w", stderr);
    logger.AddOutput(std::wcout);
#endif
    logger.AddOutput(get_log_file_path());
    logger.Log("Program Started");

    constexpr WCHAR windowClassName[] = L"TypoonWorker";

    WNDCLASSEX windowClass = {};
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.lpfnWndProc = main_wnd_proc;
    windowClass.hInstance = hInstance;
    windowClass.lpszClassName = windowClassName;
    if (!RegisterClassExW(&windowClass))
    {
        log_last_error(L"RegisterClassExW failed");
        return -1;
    }

    const HWND window = CreateWindow(windowClassName, L"Typoon Worker Process", WS_OVERLAPPEDWINDOW, 0, 0, 0, 0, nullptr, nullptr, hInstance, nullptr);
    if (!window)
    {
        log_last_error(L"CreateWindow failed");
        return -1;
    }

    show_tray_icon(std::make_tuple(hInstance, window));

    read_config_file(get_config_file_path());
    setup_trigger_tree(get_config().matchFilePath);
    start_hot_key_watcher(window);

    FileChangeWatcher configChangeWatcher{
        [window, prevMatchFilePath = std::filesystem::path{}, prevCursorPlaceholder = std::wstring{}]() mutable
        {
            read_config_file(get_config_file_path());

            if (const Config& config = get_config(); 
                prevMatchFilePath != config.matchFilePath || prevCursorPlaceholder != config.cursorPlaceholder)
            {
                prevMatchFilePath = config.matchFilePath;
                prevCursorPlaceholder = config.cursorPlaceholder;
                reconstruct_trigger_tree();
            }

            PostMessage(window, CONFIG_CHANGED_MESSAGE, 0, 0);
        }
    };
    configChangeWatcher.AddWatchingFile(get_config_file_path());

    FileChangeWatcher matchChangeWatcher{
        []()
        {
            reconstruct_trigger_tree();
        }
    };
    matchChangeWatcher.AddWatchingFile(get_config().matchFilePath);

    turn_on(window);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        // Registering hot keys should happen in the same thread as the one that has created the window.
        if (msg.message == CONFIG_CHANGED_MESSAGE)
        {
            reregister_hot_keys();
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    turn_off();

    teardown_trigger_tree();
    end_hot_key_watcher();
    remove_tray_icon();

#ifdef _DEBUG
    fclose(fDummy);
    FreeConsole();
#endif

    ExitProcess(static_cast<int>(msg.wParam));
}
