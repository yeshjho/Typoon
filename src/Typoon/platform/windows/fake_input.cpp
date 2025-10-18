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
    std::vector<INPUT> inputsToSend;
    inputsToSend.reserve(inputs.size() * 2);

    for (const auto& [type, letter] : inputs)
    {
        switch (type)
        {
        case FakeInput::EType::LETTER:
        {
            INPUT keyInput = {};
            keyInput.type = INPUT_KEYBOARD;
            keyInput.ki.wScan = letter;
            keyInput.ki.dwFlags = KEYEVENTF_UNICODE;
            keyInput.ki.dwExtraInfo = FAKE_INPUT_EXTRA_INFO_CONSTANT;
            inputsToSend.emplace_back(keyInput);
            
            keyInput.ki.dwFlags |= KEYEVENTF_KEYUP;
            inputsToSend.emplace_back(keyInput);
            break;
        }

        case FakeInput::EType::LETTER_AS_KEY:
        {
            INPUT shiftInput = {};
            shiftInput.type = INPUT_KEYBOARD;
            shiftInput.ki.dwExtraInfo = FAKE_INPUT_EXTRA_INFO_CONSTANT;

            const bool needShift = is_cased_alpha(letter) && (static_cast<bool>(std::iswupper(letter)) ^ isCapsLockOn);

            /* issue #1: Korean letter needs a shift key to type with keep_composite: true doesn't work correctly when the right shift is held
             * Originally, it only dealt with the VK_SHIFT.
             * But apparently most, if not all, applications distinguish between left and right shifts, so we're dealing with them separately.
             */
            const bool isLShiftPressed = GetAsyncKeyState(VK_LSHIFT) & 0x8000;
            const bool isRShiftPressed = GetAsyncKeyState(VK_RSHIFT) & 0x8000;
            const bool isAnyShiftPressed = isLShiftPressed || isRShiftPressed;

            const auto lambdaAddShiftKeys = [&inputsToSend, isLShiftPressed, isRShiftPressed](INPUT input)
                {
                    if (isLShiftPressed)
                    {
                        input.ki.wVk = VK_LSHIFT;
                        inputsToSend.emplace_back(input);
                    }
                    if (isRShiftPressed)
                    {
                        input.ki.wVk = VK_RSHIFT;
                        inputsToSend.emplace_back(input);
                    }
                };

            if (needShift != isAnyShiftPressed)
            {
                if (needShift)
                {
                    shiftInput.ki.wVk = VK_SHIFT;
                    inputsToSend.emplace_back(shiftInput);
                }
                else
                {
                    shiftInput.ki.dwFlags = KEYEVENTF_KEYUP;
                    lambdaAddShiftKeys(shiftInput);
                }
            }

            INPUT keyInput = {};
            keyInput.type = INPUT_KEYBOARD;
            keyInput.ki.wVk = std::towupper(letter);
            keyInput.ki.dwExtraInfo = FAKE_INPUT_EXTRA_INFO_CONSTANT;
            inputsToSend.emplace_back(keyInput);

            keyInput.ki.dwFlags = KEYEVENTF_KEYUP;
            inputsToSend.emplace_back(keyInput);
            
            if (needShift != isAnyShiftPressed)
            {
                if (needShift)
                {
                    shiftInput.ki.dwFlags = KEYEVENTF_KEYUP;
                    inputsToSend.emplace_back(shiftInput);
                }
                else
                {
                    shiftInput.ki.dwFlags = 0;
                    lambdaAddShiftKeys(shiftInput);
                }
            }
            break;
        }

        case FakeInput::EType::KEY:
        {
            INPUT keyInput = {};
            keyInput.type = INPUT_KEYBOARD;
            keyInput.ki.wVk = letter;
            keyInput.ki.dwExtraInfo = FAKE_INPUT_EXTRA_INFO_CONSTANT;
            inputsToSend.emplace_back(keyInput);

            keyInput.ki.dwFlags = KEYEVENTF_KEYUP;
            inputsToSend.emplace_back(keyInput);
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

            inputsToSend.emplace_back(ctrlInput);
            inputsToSend.emplace_back(vInput);

            vInput.ki.dwFlags = KEYEVENTF_KEYUP;
            inputsToSend.emplace_back(vInput);

            ctrlInput.ki.dwFlags = KEYEVENTF_KEYUP;
            inputsToSend.emplace_back(ctrlInput);
            break;
        }

        default:
            std::unreachable();
        }
    }

    if (const UINT uSent = SendInput(static_cast<UINT>(inputsToSend.size()), inputsToSend.data(), sizeof(INPUT));
        uSent != inputsToSend.size()) [[unlikely]]
    {
        log_last_error(L"Sending inputs failed:");
    }

    for (const auto& windowsInput : inputsToSend)
    {
        if (windowsInput.ki.dwFlags & KEYEVENTF_KEYUP)
        {
            continue;
        }
        logger.Log(ELogLevel::DEBUG, "Sent", windowsInput.ki.wVk, windowsInput.ki.wScan);
    }
}
