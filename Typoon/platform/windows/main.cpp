#include <Windows.h>

#include "../../low_level/file_change_watcher.h"
#include "../../low_level/filesystem.h"
#include "../../low_level/hotkey.h"
#include "../../low_level/tray_icon.h"

#include "../../common/common.h"
#include "../../match/trigger_tree.h"
#include "../../utils/config.h"
#include "../../utils/logger.h"

#include "wnd_proc.h"


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
        logger.Log(ELogLevel::FATAL, "RegisterClassExW failed:", std::system_category().message(static_cast<int>(GetLastError())));
        return -1;
    }

    const HWND window = CreateWindow(windowClassName, L"Typoon Worker Process", WS_OVERLAPPEDWINDOW, 0, 0, 0, 0, nullptr, nullptr, hInstance, nullptr);
    if (!window)
    {
        logger.Log(ELogLevel::FATAL, "CreateWindow failed:", std::system_category().message(static_cast<int>(GetLastError())));
        return -1;
    }

    show_tray_icon(std::make_tuple(hInstance, window));
    start_hot_key_watcher({ { EHotKeyType::TOGGLE_ON_OFF, EKey::NUM_1, EModifierKey::CONTROL | EModifierKey::ALT } }, window);

    read_config_file(get_config_file_path());
    setup_trigger_tree(get_config().matchFilePath);

    FileChangeWatcher configChangeWatcher{
        [prevMatchFilePath = std::filesystem::path{}, prevCursorPlaceholder = std::wstring{}]() mutable
        {
            read_config_file(get_config_file_path());

            if (const Config& config = get_config(); 
                prevMatchFilePath != config.matchFilePath || prevCursorPlaceholder != config.cursorPlaceholder)
            {
                prevMatchFilePath = config.matchFilePath;
                prevCursorPlaceholder = config.cursorPlaceholder;
                reconstruct_trigger_tree();
            }
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
