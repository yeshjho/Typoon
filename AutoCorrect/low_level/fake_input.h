#pragma once
#include <vector>


struct FakeInput
{
    enum class EType
    {
        LETTER,
        BACKSPACE,
        KEY,
        TOGGLE_HANGEUL,
    };

    EType type = EType::LETTER;
    wchar_t letter = 0;
};


constexpr int FAKE_INPUT_EXTRA_INFO_CONSTANT = 628;

void send_fake_inputs(const std::vector<FakeInput>& inputs, bool isCapsLockOn = false);
