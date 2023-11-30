#include <Windows.h>

#include "../../low_level/clipboard.h"
#include "../../low_level/file_change_watcher.h"
#include "../../low_level/filesystem.h"
#include "../../low_level/hotkey.h"
#include "../../low_level/tray_icon.h"

#include "../../common/common.h"
#include "../../match/trigger_tree.h"
#include "../../utils/config.h"

#include "log.h"
#include "wnd_proc.h"


int wWinMain(HINSTANCE hInstance, [[maybe_unused]] HINSTANCE hPrevInstance, [[maybe_unused]] LPWSTR cmdLine, [[maybe_unused]] int cmdShow)
try
{
    // Prevent multiple instances
    CreateMutex(nullptr, false, L"Typoon_{91CC86BD-3107-4BFB-88E2-0A9A1280AB4A}");
    if (const DWORD error = GetLastError();
        error == ERROR_ALREADY_EXISTS || error == ERROR_ACCESS_DENIED)
    {
        logger.Log("Typoon is already running. Terminating...");
        return 0;
    }

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
        log_last_error(L"RegisterClassExW failed", ELogLevel::FATAL);
        return -1;
    }

    const HWND window = CreateWindow(windowClassName, L"Typoon Worker Process", WS_OVERLAPPEDWINDOW, 0, 0, 0, 0, nullptr, nullptr, hInstance, nullptr);
    if (!window)
    {
        log_last_error(L"CreateWindow failed", ELogLevel::FATAL);
        return -1;
    }

    if (get_app_data_path().empty())
    {
        return -1;
    }

    show_tray_icon(std::make_tuple(hInstance, window));

    read_config_file(get_config_file_path());
    setup_trigger_tree(get_config().matchFilePath);
    start_hot_key_watcher(window);

    FileChangeWatcher configChangeWatcher{
        [window, prevMatchFilePath = get_config().matchFilePath, prevCursorPlaceholder = get_config().cursorPlaceholder](const std::filesystem::path&) mutable
        {
            read_config_file(get_config_file_path());

            const Config& config = get_config();
            if (const bool didMatchFilePathChange = prevMatchFilePath != config.matchFilePath;
                didMatchFilePathChange || prevCursorPlaceholder != config.cursorPlaceholder)
            {
                prevMatchFilePath = config.matchFilePath;
                prevCursorPlaceholder = config.cursorPlaceholder;
                if (didMatchFilePathChange)
                {
                    reconstruct_trigger_tree_with(config.matchFilePath);
                }
                else
                {
                    reconstruct_trigger_tree();
                }
            }

            PostMessage(window, CONFIG_CHANGED_MESSAGE, 0, 0);

            if (get_config().notifyConfigLoad)
            {
                show_notification(L"Config File Load Complete", L"The configurations are updated", true);
            }
        }
    };
    configChangeWatcher.AddWatchingFile(get_config_file_path());

    FileChangeWatcher matchChangeWatcher;
    std::function<void(const std::filesystem::path&)> matchFileChangeCallback;
    matchFileChangeCallback = [&matchChangeWatcher, &matchFileChangeCallback](const std::filesystem::path&)
    {
        reconstruct_trigger_tree({}, [&matchChangeWatcher, &matchFileChangeCallback]()
            {
                matchChangeWatcher.Reset();
                for (const std::filesystem::path& file : match_files_in_use)
                {
                    matchChangeWatcher.AddWatchingFile(file);
                }
            }
        );
    };
    matchFileChangeCallback({});
    matchChangeWatcher.SetOnChanged(matchFileChangeCallback);

    if (!turn_on(window))
    {
        return -1;
    }

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        switch (msg.message)
        {
        // Registering hot keys should happen in the same thread as the one that has created the window.
        case CONFIG_CHANGED_MESSAGE:
            reregister_hot_keys();
            break;

        case CLIPBOARD_PASTE_DONE_MESSAGE:
            pop_clipboard_state_with_delay([seq = GetClipboardSequenceNumber()]() { return seq == GetClipboardSequenceNumber(); });
            break;

        default:
            break;
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

    return static_cast<int>(msg.wParam);
}
catch (const std::exception& e)
{
    logger.Log(ELogLevel::FATAL, "Exception in main", e.what());
    show_notification(L"Fatal Exception",
        L"An exception occurred and Typoon is shutting down.\nPlease report this with a log file.", false);
    return -1;
}
catch (...)
{
    logger.Log(ELogLevel::FATAL, "Unknown exception in main");
    show_notification(L"Fatal Exception",
        L"An exception occurred and Typoon is shutting down.\nPlease report this with a log file.", false);
    return -1;
}