#include "../../low_level/fake_input.h"

#include <cwctype>

#include <Windows.h>

#include "../../utils/logger.h"
#include "../../utils/string.h"


void send_fake_inputs(const std::vector<FakeInput>& inputs, bool isCapsLockOn)
{
    std::vector<INPUT> windowsInputs;
    windowsInputs.reserve(inputs.size() * 2);

    for (const auto& [type, letter] : inputs)
    {
        switch (type)
        {
        case FakeInput::EType::LETTER:
        {
            INPUT windowsInput = {};
            windowsInput.type = INPUT_KEYBOARD;
            windowsInput.ki.wScan = letter;
            windowsInput.ki.dwFlags = KEYEVENTF_UNICODE;
            windowsInput.ki.dwExtraInfo = FAKE_INPUT_EXTRA_INFO_CONSTANT;
            windowsInputs.emplace_back(windowsInput);
            
            windowsInput.ki.dwFlags |= KEYEVENTF_KEYUP;
            windowsInputs.emplace_back(windowsInput);
            break;
        }

        case FakeInput::EType::BACKSPACE:
        {
            INPUT windowsInput = {};
            windowsInput.type = INPUT_KEYBOARD;
            windowsInput.ki.wVk = VK_BACK;
            windowsInput.ki.dwExtraInfo = FAKE_INPUT_EXTRA_INFO_CONSTANT;
            windowsInputs.emplace_back(windowsInput);

            windowsInput.ki.dwFlags = KEYEVENTF_KEYUP;
            windowsInputs.emplace_back(windowsInput);
            break;
        }

        case FakeInput::EType::KEY:
        {
            INPUT shiftInput = {};
            shiftInput.type = INPUT_KEYBOARD;
            shiftInput.ki.wVk = VK_SHIFT;
            shiftInput.ki.dwExtraInfo = FAKE_INPUT_EXTRA_INFO_CONSTANT;

            const bool needShift = is_cased_alpha(letter) && (static_cast<bool>(std::iswupper(letter)) ^ isCapsLockOn);
            if (needShift)
            {
                windowsInputs.emplace_back(shiftInput);
            }

            INPUT windowsInput = {};
            windowsInput.type = INPUT_KEYBOARD;
            windowsInput.ki.wVk = std::towupper(letter);
            windowsInput.ki.dwExtraInfo = FAKE_INPUT_EXTRA_INFO_CONSTANT;
            windowsInputs.emplace_back(windowsInput);

            windowsInput.ki.dwFlags = KEYEVENTF_KEYUP;
            windowsInputs.emplace_back(windowsInput);

            if (needShift)
            {
                shiftInput.ki.dwFlags = KEYEVENTF_KEYUP;
                windowsInputs.emplace_back(shiftInput);
            }
            break;
        }

        case FakeInput::EType::TOGGLE_HANGEUL:
        {
            INPUT windowsInput = {};
            windowsInput.type = INPUT_KEYBOARD;
            windowsInput.ki.wVk = VK_HANGEUL;
            windowsInput.ki.dwExtraInfo = FAKE_INPUT_EXTRA_INFO_CONSTANT;
            windowsInputs.emplace_back(windowsInput);

            windowsInput.ki.dwFlags = KEYEVENTF_KEYUP;
            windowsInputs.emplace_back(windowsInput);
            break;
        }

        default:
            std::unreachable();
        }
    }

    for (const auto& windowsInput : windowsInputs)
    {
        g_console_logger.Log(ELogLevel::DEBUG, "Sending", windowsInput.ki.wVk, windowsInput.ki.wScan);
    }
    
    if (const UINT uSent = SendInput(static_cast<UINT>(windowsInputs.size()), windowsInputs.data(), sizeof(INPUT));
        uSent != windowsInputs.size()) [[unlikely]]
    {
        g_console_logger.Log(ELogLevel::ERROR, "Sending inputs failed:", std::system_category().message(static_cast<int>(GetLastError())));
    }
}
