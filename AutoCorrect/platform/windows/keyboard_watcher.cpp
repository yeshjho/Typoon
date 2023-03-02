#include "../../low_level/keyboard_watcher.h"

#include <system_error>
#include <thread>
#include <Windows.h>

#include "../../input_multicast/input_multicast.h"
#include "../../utils/logger.h"


static LRESULT CALLBACK low_level_keyboard_proc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (wParam != WM_KEYDOWN || lParam == NULL)
    {
        return CallNextHookEx(nullptr, nCode, wParam, lParam);
    }

    const auto param = reinterpret_cast<LPKBDLLHOOKSTRUCT>(lParam);
    unsigned char keyboardState[256] = { 0, };
    // ToUnicodeEx produces in UTF-16, so 2 wchar_t's are enough.
    wchar_t characters[2] = { 0, };
    
    GetKeyState(0);  // GetKeyboardState doesn't fetch control keys such as Shift, CapsLock, etc. without this call.
    if (!GetKeyboardState(keyboardState) || (keyboardState[VK_LWIN] & 0x80))  // Even when the Windows key is pressed, ToAsciiEx will return the character of the key, thus filtering out.
    {
        return CallNextHookEx(nullptr, nCode, wParam, lParam);
    }

    const HWND foregroundWindow = GetForegroundWindow();
    if (const int result = ToUnicodeEx(param->vkCode, param->scanCode, keyboardState, characters, static_cast<int>(std::size(characters)), 0, GetKeyboardLayout(GetWindowThreadProcessId(foregroundWindow, nullptr)));
        result <= 0)
    {
        return CallNextHookEx(nullptr, nCode, wParam, lParam);
    }

    for (wchar_t character : characters)
    {
        if (character == '\0')
        {
            continue;
        }

        if (const bool isLowerAlphabet = 'a' <= character && character <= 'z', isUpperAlphabet = 'A' <= character && character <= 'Z';
            isLowerAlphabet || isUpperAlphabet)
        {
            if (const HWND defaultImeWindow = ImmGetDefaultIMEWnd(foregroundWindow);
                SendMessage(defaultImeWindow, WM_IME_CONTROL, 0x0005/*IMC_GETOPENSTATUS*/, 0))  // is Hangul on
                // keyboardState[VK_HANGUL] doesn't work since it only stores the state of the last key pressed, not the current state(toggled), unlike VK_CAPITAL.
                // Maybe should check the keyboard layout?
            {
                if (keyboardState[VK_CAPITAL] & 0x1)
                {
                    if (isLowerAlphabet)
                    {
                        character -= 'a' - 'A';
                    }
                    else
                    {
                        character += 'a' - 'A';
                    }
                }

                // Support only two-set keyboard layout for now.
                constexpr wchar_t lowerAlphabetToHangul[] = L"ㅁㅠㅊㅇㄷㄹㅎㅗㅑㅓㅏㅣㅡㅜㅐㅔㅂㄱㄴㅅㅕㅍㅈㅌㅛㅋ";
                constexpr wchar_t upperAlphabetToHangul[] = L"ㅁㅠㅊㅇㄸㄹㅎㅗㅑㅓㅏㅣㅡㅜㅒㅖㅃㄲㄴㅆㅕㅍㅉㅌㅛㅋ";
                if (isLowerAlphabet)
                {
                    character = lowerAlphabetToHangul[character - L'a'];
                }
                else
                {
                    character = upperAlphabetToHangul[character - L'A'];
                }
            }
        }

        multicast_input(character);
    }

    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}


HHOOK keyboard_watcher_hook = nullptr;
std::jthread keyboard_watcher_thread;

void start_keyboard_watcher()
{
    if (keyboard_watcher_hook || keyboard_watcher_thread.joinable()) [[unlikely]]
    {
        g_console_logger.Log("Keyboard watcher is already running.", ELogLevel::WARNING);
        return;
    }

    keyboard_watcher_thread = std::jthread{ [](const std::stop_token& stopToken)
    {
        keyboard_watcher_hook = SetWindowsHookEx(WH_KEYBOARD_LL, low_level_keyboard_proc, nullptr, 0);
        if (!keyboard_watcher_hook) [[unlikely]]
        {
            g_console_logger.Log(ELogLevel::FATAL, "WH_KEYBOARD_LL hook failed:", std::system_category().message(static_cast<int>(GetLastError())));
            std::exit(-1);  // It's fatal anyway, thread safety is not needed.
        }

        MSG msg;
        while (true)
        {
            if (stopToken.stop_requested()) [[unlikely]]
            {
                break;
            }

            PeekMessage(&msg, nullptr, 0, 0, 0);
            if (msg.message == WM_QUIT) [[unlikely]]
            {
                break;
            }
        }
    } };
}

void end_keyboard_watcher()
{
    if (keyboard_watcher_hook) [[likely]]
    {
        UnhookWindowsHookEx(keyboard_watcher_hook);
    }

    keyboard_watcher_thread.request_stop();
    if (keyboard_watcher_thread.joinable())
    {
        keyboard_watcher_thread.join();
    }
}

