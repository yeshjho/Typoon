#include <Windows.h>

#include "../../low_level/input_watcher.h"
#include "../../low_level/file_change_watcher.h"

#include "../../imm/imm_simulator.h"
#include "../../match/trigger_tree.h"
#include "../../utils/logger.h"


LRESULT dummy_wnd_proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    return DefWindowProc(hWnd, msg, wParam, lParam);
}


int wWinMain(HINSTANCE hInstance, [[maybe_unused]] HINSTANCE hPrevInstance, [[maybe_unused]] LPWSTR cmdLine, [[maybe_unused]] int cmdShow)
{
    MSG msg;

    constexpr WCHAR windowClassName[] = L"AutoCorrectWorker";

    WNDCLASSEX windowClass = {};
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.lpfnWndProc = dummy_wnd_proc;  // CreateWindow calls wndProc, so setting to a dummy.
                                               // Will be replaced in the input watcher.
    windowClass.hInstance = hInstance;
    windowClass.lpszClassName = windowClassName;
    if (!RegisterClassExW(&windowClass))
    {
        g_console_logger.Log(ELogLevel::FATAL, "RegisterClassExW failed:", std::system_category().message(static_cast<int>(GetLastError())));
        return -1;
    }

    const HWND window = CreateWindow(windowClassName, L"AutoCorrect Worker Process", WS_OVERLAPPEDWINDOW, 0, 0, 0, 0, nullptr, nullptr, hInstance, nullptr);
    if (!window)
    {
        g_console_logger.Log(ELogLevel::FATAL, "CreateWindow failed:", std::system_category().message(static_cast<int>(GetLastError())));
        return -1;
    }

#ifdef _DEBUG
    AllocConsole();
    FILE* fDummy;
    freopen_s(&fDummy, "CONOUT$", "w", stdout);
    freopen_s(&fDummy, "CONOUT$", "w", stderr);
#endif

    start_input_watcher(window);
    start_file_change_watcher("C:/Users/yeshj/Desktop/folders/Visual Studio/AutoCorrect/AutoCorrect/data/", []()
        {
            reconstruct_trigger_tree();
        });
    setup_imm_simulator();
    setup_trigger_tree("C:/Users/yeshj/Desktop/folders/Visual Studio/AutoCorrect/AutoCorrect/data/test.json5");

    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    end_input_watcher();
    end_file_change_watcher();
    teardown_imm_simulator();

    return static_cast<int>(msg.wParam);
}
