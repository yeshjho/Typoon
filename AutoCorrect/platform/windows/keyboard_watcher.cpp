#include "../../low_level/keyboard_watcher.h"

#include <cwctype>
#include <system_error>
#include <thread>

#include <Windows.h>
#include <hidusage.h>

#include "../../imm/imm_simulator.h"
#include "../../utils/logger.h"


LRESULT CALLBACK low_level_keyboard_proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg != WM_INPUT)
    {
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }

    // Will be always in the background, no need to check the wParam.
    RAWINPUT inputData;
    UINT size;
    if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, &inputData, &size, sizeof(RAWINPUTHEADER))
        == static_cast<UINT>(-1))
    {
        g_console_logger.Log(ELogLevel::ERROR, "GetRawInputData failed:", std::system_category().message(static_cast<int>(GetLastError())));
        return 0;
    }

    switch (inputData.header.dwType)
    {
    case RIM_TYPEKEYBOARD:
    {
        const RAWKEYBOARD& keyboardData = inputData.data.keyboard;

        if (keyboardData.Message != WM_KEYDOWN)
        {
            break;
        }

        unsigned char keyboardState[256] = { 0, };
        // ToUnicodeEx produces in UTF-16, so 2 wchar_t's are enough.
        wchar_t characters[2] = { 0, };

        GetKeyState(0);  // GetKeyboardState doesn't fetch control keys such as Shift, CapsLock, etc. without this call.
        if (!GetKeyboardState(keyboardState) ||
            // Even when the Windows key is pressed, ToUnicodeEx will return the character of the key, thus filtering out.
            // Also, the control key + a-z => 1~26, filter out.
            (keyboardState[VK_LWIN] & 0x80) || (keyboardState[VK_RWIN] & 0x80) || (keyboardState[VK_CONTROL] & 0x80))
        {
            break;
        }
        // TODO: Alt key can affect the following key even when it's not down currently.

        const HWND foregroundWindow = GetForegroundWindow();
        if (const int result = ToUnicodeEx(keyboardData.VKey, keyboardData.MakeCode, keyboardState, characters, static_cast<int>(std::size(characters)), 0, GetKeyboardLayout(GetWindowThreadProcessId(foregroundWindow, nullptr)));
            result <= 0)
        {
            break;
        }
        
        // We currently don't support characters need more than 2 bytes
        wchar_t character = characters[0];
        if (const bool isLowerAlphabet = 'a' <= character && character <= 'z', isUpperAlphabet = 'A' <= character && character <= 'Z';
            isLowerAlphabet || isUpperAlphabet)
        {
            // is Hangul on
            if (const HWND defaultImeWindow = ImmGetDefaultIMEWnd(foregroundWindow);
                SendMessage(defaultImeWindow, WM_IME_CONTROL, 0x0005/*IMC_GETOPENSTATUS*/, 0))
                // keyboardState[VK_HANGUL] doesn't work since it only stores the state of the last key pressed, not the current state(toggled), unlike VK_CAPITAL.
                // Maybe should check the keyboard layout?
            {
                if (keyboardState[VK_CAPITAL] & 0x1)
                {
                    // Flip the case since the CapsLock can't affect the Korean letters, but we use the English letters to convert to Korean letters.
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

        if (std::iswprint(character) || std::iswspace(character) || character == L'\b')
        {
            send_raw_input_to_imm_simulator(character);
        }
        break;
    }

    case RIM_TYPEMOUSE:
    {
        break;
    }

    case RIM_TYPEHID:
    {
        break;
    }

    default:
        std::unreachable();
    }

    return 0;
}


void start_keyboard_watcher(const std::any& data)
{
    static bool didStart = false;
    if (didStart)
    {
        g_console_logger.Log("Keyboard watcher is already running.", ELogLevel::WARNING);
        return;
    }
    didStart = true;

    const HWND hWnd = std::any_cast<HWND>(data);

    if (!SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(low_level_keyboard_proc)))
    {
        g_console_logger.Log(ELogLevel::FATAL, "SetWindowLongPtr failed:", std::system_category().message(static_cast<int>(GetLastError())));
        std::exit(-1);  // It's fatal anyway, thread safety
    }

    RAWINPUTDEVICE rid[2];

    rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
    rid[0].usUsage = HID_USAGE_GENERIC_KEYBOARD;
    rid[0].dwFlags = RIDEV_NOLEGACY | RIDEV_INPUTSINK;
    rid[0].hwndTarget = hWnd;

    rid[1].usUsagePage = HID_USAGE_PAGE_GENERIC;
    rid[1].usUsage = HID_USAGE_GENERIC_MOUSE;
    rid[1].dwFlags = RIDEV_INPUTSINK;
    rid[1].hwndTarget = hWnd;

    if (!RegisterRawInputDevices(rid, 1, sizeof(rid[0])))
    {
        g_console_logger.Log(ELogLevel::FATAL, "RegisterRawInputDevices failed:", std::system_category().message(static_cast<int>(GetLastError())));
        std::exit(-1);  // It's fatal anyway, thread safety is not needed.
    }
}

void end_keyboard_watcher()
{
}

