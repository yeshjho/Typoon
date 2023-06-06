#include "../../low_level/input_watcher.h"

#include <cwctype>

#include <Windows.h>
#include <hidusage.h>

#include "../../imm/imm_simulator.h"
#include "../../input_multicast/input_multicast.h"
#include "../../low_level/fake_input.h"
#include "../../utils/logger.h"
#include "../../utils/string.h"
#include "log.h"
#include "wnd_proc.h"


std::optional<LRESULT> input_proc([[maybe_unused]] HWND hWnd, UINT msg, [[maybe_unused]] WPARAM wParam, LPARAM lParam)
{
    if (msg != WM_INPUT)
    {
        return std::nullopt;
    }

    // Will be always in the background, no need to check the wParam.
    RAWINPUT inputData;
    UINT size = sizeof(RAWINPUT);
    if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, &inputData, &size, sizeof(RAWINPUTHEADER))
        == static_cast<UINT>(-1))
    {
        log_last_error(L"GetRawInputData failed:");
        return 0;
    }

    switch (inputData.header.dwType)
    {
    case RIM_TYPEKEYBOARD:
    {
        const RAWKEYBOARD& keyboardData = inputData.data.keyboard;

        if (keyboardData.ExtraInformation == FAKE_INPUT_EXTRA_INFO_CONSTANT)
        {
            break;
        }

        if (keyboardData.Message == WM_SYSKEYUP)
        {
            // Alt + Other key
            imm_simulator.ClearComposition();
            multicast_input({}, 0, true);
            break;
        }

        if (keyboardData.Message != WM_KEYDOWN)
        {
            break;
        }

        const USHORT vKey = keyboardData.VKey;
        if (vKey == VK_SHIFT || vKey == VK_CONTROL || vKey == VK_SNAPSHOT)
        {
            // These alone don't affect the stroke at all.
            break;
        }

        if (vKey == VK_CAPITAL || vKey == VK_SCROLL || vKey == VK_NUMLOCK || 
            vKey == VK_ESCAPE || vKey == VK_PAUSE || vKey == VK_INSERT || vKey == VK_PRIOR || vKey == VK_NEXT ||
            vKey == VK_HANGEUL)
        {
            // These don't affect the stroke itself, but finishes the current composition.
            imm_simulator.EmitAndClearCurrentComposite();
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
            imm_simulator.ClearComposition();
            multicast_input({}, 0, true);
            break;
        }
        // TODO: Alt key can affect the following key even when it's not down currently.

        const HWND foregroundWindow = GetForegroundWindow();
        if (const int result = ToUnicodeEx(vKey, keyboardData.MakeCode, keyboardState, characters, static_cast<int>(std::size(characters)), 0, GetKeyboardLayout(GetWindowThreadProcessId(foregroundWindow, nullptr)));
            result <= 0)
        {
            imm_simulator.ClearComposition();
            multicast_input({}, 0, true);
            break;
        }
        
        // We currently don't support characters need more than 2 bytes
        wchar_t character = characters[0];
        if (const bool isLowerAlphabet = 'a' <= character && character <= 'z', isUpperAlphabet = 'A' <= character && character <= 'Z';
            isLowerAlphabet || isUpperAlphabet)
        {
            const HWND defaultImeWindow = ImmGetDefaultIMEWnd(foregroundWindow);
            is_hangeul_on = static_cast<bool>(SendMessage(defaultImeWindow, WM_IME_CONTROL, 0x0005/*IMC_GETOPENSTATUS*/, 0));
            // keyboardState[VK_HANGUL] doesn't work since it only stores the state of the last key pressed, not the current state(toggled), unlike VK_CAPITAL.
            // Maybe should check the keyboard layout?
            if (is_hangeul_on)
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

                character = alphabet_to_hangeul({ &character, 1 })[0];
            }
        }

        if (std::iswprint(character) || std::iswspace(character) || character == L'\b')
        {
            imm_simulator.AddLetter(character);
        }
        break;
    }

    case RIM_TYPEMOUSE:
    {
        const RAWMOUSE& mouseData = inputData.data.mouse;

        if (mouseData.usButtonFlags & 
            (RI_MOUSE_BUTTON_1_DOWN | RI_MOUSE_BUTTON_2_DOWN | 
                RI_MOUSE_BUTTON_4_DOWN | RI_MOUSE_BUTTON_5_DOWN)
            )
        {
            imm_simulator.ClearComposition();
            multicast_input({}, 0, true);
        }

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


void start_input_watcher(const std::any& data)
{
    wnd_proc_functions.emplace_back("input", input_proc);

    const HWND hWnd = std::any_cast<HWND>(data);
    RAWINPUTDEVICE rid[2];

    rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
    rid[0].usUsage = HID_USAGE_GENERIC_KEYBOARD;
    rid[0].dwFlags = RIDEV_NOLEGACY | RIDEV_INPUTSINK;
    rid[0].hwndTarget = hWnd;

    rid[1].usUsagePage = HID_USAGE_PAGE_GENERIC;
    rid[1].usUsage = HID_USAGE_GENERIC_MOUSE;
    rid[1].dwFlags = RIDEV_INPUTSINK;
    rid[1].hwndTarget = hWnd;

    if (!RegisterRawInputDevices(rid, static_cast<UINT>(std::size(rid)), sizeof(rid[0])))
    {
        log_last_error(L"RegisterRawInputDevices failed:");
        ExitProcess(1);
    }
}


void end_input_watcher()
{
    std::erase_if(wnd_proc_functions, [](const std::pair<std::string, WndProcFunc>& pair) { return pair.first == "input"; });
}

