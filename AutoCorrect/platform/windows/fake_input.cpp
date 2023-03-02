#include "../../low_level/fake_input.h"

#include <Windows.h>

#include "../../utils/logger.h"


void send_fake_inputs(const std::vector<FakeInput>& inputs)
{
    std::vector<INPUT> windowsInputs;
    windowsInputs.reserve(inputs.size() * 2);

    for (const auto& [type, letter] : inputs)
    {
        switch (type)
        {
        case FakeInput::EType::LETTER:
        {
            INPUT windowsInput = { };
            windowsInput.type = INPUT_KEYBOARD;
            windowsInput.ki.wVk = 0;
            windowsInput.ki.wScan = letter;
            windowsInput.ki.dwFlags = KEYEVENTF_UNICODE;
            windowsInputs.emplace_back(windowsInput);
            
            windowsInput.ki.dwFlags |= KEYEVENTF_KEYUP;
            windowsInputs.emplace_back(windowsInput);
            break;
        }

        case FakeInput::EType::BACKSPACE:
        {
            INPUT windowsInput = { };
            windowsInput.type = INPUT_KEYBOARD;
            windowsInput.ki.wVk = VK_BACK;
            windowsInputs.emplace_back(windowsInput);

            windowsInput.ki.dwFlags = KEYEVENTF_KEYUP;
            windowsInputs.emplace_back(windowsInput);
            break;
        }

        default:
            std::unreachable();
        }
    }

    if (const UINT uSent = SendInput(static_cast<UINT>(windowsInputs.size()), windowsInputs.data(), sizeof(INPUT));
        uSent != windowsInputs.size()) [[unlikely]]
    {
        g_console_logger.Log(ELogLevel::ERROR, "Sending inputs failed:", std::system_category().message(static_cast<int>(GetLastError())));
    }
}
