#pragma once
#include <vector>


struct FakeInput
{
    enum class EType
    {
        LETTER,
        LETTER_AS_KEY,
        KEY,
    };

    EType type = EType::LETTER;
    wchar_t letter = 0;

    static const wchar_t BACKSPACE_KEY;
    static const wchar_t TOGGLE_HANGEUL_KEY;
    static const wchar_t LEFT_ARROW_KEY;
    static const wchar_t ENTER_KEY;
};


constexpr int FAKE_INPUT_EXTRA_INFO_CONSTANT = 628;

void send_fake_inputs(const std::vector<FakeInput>& inputs, bool isCapsLockOn = false);
