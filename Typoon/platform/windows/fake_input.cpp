#include "../../low_level/fake_input.h"

#include <cwctype>

#include <Windows.h>

#include "../../utils/string.h"
#include "log.h"


const wchar_t FakeInput::BACKSPACE_KEY = VK_BACK;
const wchar_t FakeInput::TOGGLE_HANGEUL_KEY = VK_HANGUL;
const wchar_t FakeInput::LEFT_ARROW_KEY = VK_LEFT;
const wchar_t FakeInput::ENTER_KEY = VK_RETURN;


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

        case FakeInput::EType::LETTER_AS_KEY:
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

        case FakeInput::EType::KEY:
        {
            INPUT windowsInput = {};
            windowsInput.type = INPUT_KEYBOARD;
            windowsInput.ki.wVk = letter;
            windowsInput.ki.dwExtraInfo = FAKE_INPUT_EXTRA_INFO_CONSTANT;
            windowsInputs.emplace_back(windowsInput);

            windowsInput.ki.dwFlags = KEYEVENTF_KEYUP;
            windowsInputs.emplace_back(windowsInput);
            break;
        }

        case FakeInput::EType::HOT_KEY_PASTE:
        {
            INPUT ctrlInput = {};
            ctrlInput.type = INPUT_KEYBOARD;
            ctrlInput.ki.wVk = VK_CONTROL;
            ctrlInput.ki.dwExtraInfo = FAKE_INPUT_EXTRA_INFO_CONSTANT;

            INPUT vInput = {};
            vInput.type = INPUT_KEYBOARD;
            vInput.ki.wVk = 'V';
            vInput.ki.dwExtraInfo = FAKE_INPUT_EXTRA_INFO_CONSTANT;

            windowsInputs.emplace_back(ctrlInput);
            windowsInputs.emplace_back(vInput);

            vInput.ki.dwFlags = KEYEVENTF_KEYUP;
            windowsInputs.emplace_back(vInput);

            ctrlInput.ki.dwFlags = KEYEVENTF_KEYUP;
            windowsInputs.emplace_back(ctrlInput);
            break;
        }

        default:
            std::unreachable();
        }
    }

    if (const UINT uSent = SendInput(static_cast<UINT>(windowsInputs.size()), windowsInputs.data(), sizeof(INPUT));
        uSent != windowsInputs.size()) [[unlikely]]
    {
        log_last_error(L"Sending inputs failed:");
    }

    for (const auto& windowsInput : windowsInputs)
    {
        if (windowsInput.ki.dwFlags & KEYEVENTF_KEYUP)
        {
            continue;
        }
        logger.Log(ELogLevel::DEBUG, "Sent", windowsInput.ki.wVk, windowsInput.ki.wScan);
    }
}
