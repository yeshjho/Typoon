#include <Windows.h>

#include "../../low_level/input_watcher.h"
#include "../../low_level/file_change_watcher.h"
#include "../../low_level/filesystem.h"
#include "../../low_level/tray_icon.h"

#include "../../imm/imm_simulator.h"
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

    start_input_watcher(window);
    read_config_file(get_app_data_path() / "config.json5");
    start_file_change_watcher(get_app_data_path(), []()
        {
            read_config_file(get_app_data_path() / "config.json5");
            reconstruct_trigger_tree();
        });
    setup_imm_simulator();
    setup_trigger_tree(get_config().matchFilePath);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    end_input_watcher();
    end_file_change_watcher();
    teardown_imm_simulator();

    remove_tray_icon(window);

#ifdef _DEBUG
    fclose(fDummy);
    FreeConsole();
#endif

    return static_cast<int>(msg.wParam);
}
