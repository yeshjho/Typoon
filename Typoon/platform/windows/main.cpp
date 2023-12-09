#include <Windows.h>

#include "../../low_level/clipboard.h"
#include "../../low_level/file_change_watcher.h"
#include "../../low_level/filesystem.h"
#include "../../low_level/hotkey.h"
#include "../../low_level/tray_icon.h"
#include "../../low_level/window_focus.h"

#include "../../common/common.h"
#include "../../match/trigger_trees_per_program.h"
#include "../../parse/parse_match.h"
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
    setup_trigger_trees(get_config().matchFilePath);
    update_trigger_tree_program_overrides(get_config().programOverrides);
    start_hot_key_watcher(window);

    FileChangeWatcher matchChangeWatcher;
    const auto lambdaReinitializeMatchChangeWatcher = [&matchChangeWatcher]()
        {
            matchChangeWatcher.Reset();
            for (const auto& [file, trees] : trigger_trees_by_match_file)
            {
                if (!trees.empty())
                {
                    matchChangeWatcher.AddWatchingFile(file);
                }
            }
        };

    FileChangeWatcher configChangeWatcher{
        [window, 
        prevMatchFilePath = get_config().matchFilePath, 
        prevCursorPlaceholder = get_config().cursorPlaceholder,
        prevProgramOverrides = get_config().programOverrides,
        &lambdaReinitializeMatchChangeWatcher](const std::filesystem::path&) mutable
        {
            read_config_file(get_config_file_path());

            const Config& config = get_config();
            if (prevMatchFilePath != config.matchFilePath)
            {
                prevMatchFilePath = config.matchFilePath;
                get_trigger_tree(DEFAULT_PROGRAM_NAME)->ReconstructWith(config.matchFilePath);
            }

            if (prevCursorPlaceholder != config.cursorPlaceholder)
            {
                prevCursorPlaceholder = config.cursorPlaceholder;
                invalidate_all_matches_cache();
                reconstruct_all_trigger_trees();
            }

            if (prevProgramOverrides != config.programOverrides)
            {
                prevProgramOverrides = config.programOverrides;
                update_trigger_tree_program_overrides(config.programOverrides);
                reconstruct_all_trigger_trees(lambdaReinitializeMatchChangeWatcher);
            }

            PostMessage(window, CONFIG_CHANGED_MESSAGE, 0, 0);

            if (get_config().notifyConfigLoad)
            {
                show_notification(L"Config File Load Complete", L"The configurations are updated", true);
            }
        }
    };
    configChangeWatcher.AddWatchingFile(get_config_file_path());

    const auto onMatchFileChanged = [&lambdaReinitializeMatchChangeWatcher](const std::filesystem::path& fileChanged)
        {
            invalidate_matches_cache(fileChanged);

            const std::deque<TriggerTree*>& treesToReconstruct = trigger_trees_by_match_file[fileChanged];
            if (treesToReconstruct.empty())
            {
                return;
            }
            
            for (TriggerTree* triggerTree : treesToReconstruct | std::views::take(treesToReconstruct.size() - 1))
            {
                triggerTree->Reconstruct();
            }

            treesToReconstruct.back()->Reconstruct({}, [&lambdaReinitializeMatchChangeWatcher]()
            {
                lambdaReinitializeMatchChangeWatcher();
            });
        };
    matchChangeWatcher.SetOnChanged(onMatchFileChanged);
    reconstruct_all_trigger_trees(lambdaReinitializeMatchChangeWatcher);

    if (get_config().notifyMatchLoad)
    {
        show_notification(L"Match File Load Complete!", L"The match file is parsed and ready to go", true);
    }

    if (!turn_on(window))
    {
        return -1;
    }

    CoInitialize(nullptr);

    GUITHREADINFO gti{ .cbSize = sizeof(GUITHREADINFO) };
    if (GetGUIThreadInfo(0, &gti))
    {
        const HWND foregroundWindow = gti.hwndFocus;
        DWORD processId;
        if (GetWindowThreadProcessId(foregroundWindow, &processId))
        {
            check_for_window_focus_change(processId);
        }
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

    CoUninitialize();

    turn_off();

    teardown_trigger_trees();
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